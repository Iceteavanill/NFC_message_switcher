/*
This software was written by Fabian Steiner for the NFC_message_switcher Project.
It is provided AS IS with no liability for anything this software may or may not be the cause of.
For more information check the github readme.
This Project uses various AVR libraries aswell as the PIC bare metal reference driver
*/

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "TWI_host.h"

// EEprom Data adresses
uint8_t *EEpromIncAdr = (uint8_t *)0x00;  // Adress for incremental acces counting (gets looped around after 255)
uint8_t *EEpromInitAdr = (uint8_t *)0x01; // Adress for storage of Initialisation status

// pointer to data that should be writen to the defined registers
uint8_t EHMData = 0x00;
uint8_t GPOData = 0x82;

// functions

// func to go to sleep indefinitely -> after NFC tag has been written
void nightynighty();
// function to go to sleep but also read Interrupt and wake up
void nightyWakey();
// init the timer for periodic interrupt
void init_timer();
// harware wait(with timer and interupt)
void harwareWait();

// fuse definition
FUSES = {
    .WDTCFG = 0x00,  // WDTCFG {PERIOD=OFF, WINDOW=OFF}
	.BODCFG = 0x05, // BODCFG {SLEEP=ENABLED, ACTIVE=ENABLED, SAMPFREQ=1KHz, LVL=BODLEVEL0}
    .OSCCFG = 0x01,  // OSCCFG {FREQSEL=16MHZ, OSCLOCK=CLEAR}
    .TCD0CFG = 0x00, // TCD0CFG {CMPA=CLEAR, CMPB=CLEAR, CMPC=CLEAR, CMPD=CLEAR, CMPAEN=CLEAR, CMPBEN=CLEAR, CMPCEN=CLEAR, CMPDEN=CLEAR}
    .SYSCFG0 = 0xF6, // SYSCFG0 {EESAVE=CLEAR, RSTPINCFG=UPDI, CRCSRC=NOCRC}
    .SYSCFG1 = 0x07, // SYSCFG1 {SUT=64MS} -> must be 64ms or the Capacitor does not have enough time to charge enough
    .APPEND = 0x00,  // APPEND {APPEND=User range:  0x0 - 0xFF}
    .BOOTEND = 0x00, // BOOTEND {BOOTEND=User range:  0x0 - 0xFF}
};

LOCKBITS = 0xC5; // {LB=NOLOCK}

int main()
{
    // set clock configuration
    CLKCTRL.MCLKCTRLA &= ~CLKCTRL_CLKSEL_gm;          // clear configuration (set to 0)
    CLKCTRL.MCLKCTRLA |= CLKCTRL_CLKSEL_OSCULP32K_gc; // set clock to internal 32kHz LP clock
    CLKCTRL.MCLKCTRLB &= ~CLKCTRL_PEN_bm;             // Disable internal Clock Divider (for now))
    CLKCTRL.MCLKLOCK |= 1;                            // lock clock configuration

    // Init I2C connection
    TWI_initHost();

    // Set PA6 (GPO input) as input
    PORTA.DIRCLR |= PIN6_bm;
    // enable pullup (is needed because open drain output)
    PORTA.PIN6CTRL |= PORT_PULLUPEN_bm;

    // check if the ST25DV04 device has been Inited
    if (eeprom_read_byte(EEpromInitAdr) != 0x69) 
    {
        // Password for system access (default)
        uint8_t defaultpasswordSequence[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        bool slaveDeviceError = false;

        // Provide Password to ST25DV device
        if (!TWI_sendBytesToRegister(SLAVE_ADDRESS_SYSTEM, SysReg_PasswordAdr, &defaultpasswordSequence[0], 17))
        {
            slaveDeviceError = true;
        }
        harwareWait();

        // now that a Secure session is open -> write EH mode
        if (!TWI_sendBytesToRegister(SLAVE_ADDRESS_SYSTEM, SysReg_EH_MODE, &EHMData, 1))
        {
            slaveDeviceError = true;
        }
        harwareWait();

        // write GPO mode
        if (!TWI_sendBytesToRegister(SLAVE_ADDRESS_SYSTEM, SysReg_GPO, &GPOData, 1))
        {
            slaveDeviceError = true;
        }
        harwareWait();

        // check if writing to device was successfull
        if (slaveDeviceError)
        {
            // cant do anything stop wait here for debug
            nightynighty();
            return -1;
        }
        harwareWait();

        // check EH mode
        if (!TWI_VerifyBytes(SLAVE_ADDRESS_SYSTEM, SysReg_EH_MODE, &EHMData, 1))
        {
            slaveDeviceError = true;
        }
        harwareWait();

        // check GPO mode
        if (!TWI_VerifyBytes(SLAVE_ADDRESS_SYSTEM, SysReg_GPO, &GPOData, 1))
        {
            slaveDeviceError = true;
        }

        // if all good (writing to device was successfull)
        if (!slaveDeviceError)
        {
            // ST25dv was initialized -> mark as initialized
            eeprom_write_byte(EEpromInitAdr, 0x69);
        }
        else
        {
            // cant do anything stop wait here for debug
            nightynighty();
            return -1;
        }
    }

    uint8_t *pData;      // datapointer

    // read eeprom inc adress
    uint8_t eepromData = eeprom_read_byte(EEpromIncAdr);

    // clamp eeprom adress
    if (eepromData == 255)
    {
        eepromData = 0;
    }
    else
    {
        eepromData++;
    }

    // CHECK DATALENGHT AND SET IT CORRECTY!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // testdata -> edit for usecase
    uint8_t dat1[] = {0XE1, 0X40, 0X40, 0X00, 0X03, 0X1A, 0XD1, 0X01, 0X16, 0X54, 0X02, 0X65, 0X6E, 0X49, 0X20, 0X77, 0X61, 0X73, 0X20, 0X72, 0X75, 0X6E, 0X20, 0X20, 0X20, 0X31, 0X20, 0X74, 0X69, 0X6D, 0X65, 0X73, 0XFE};
    const uint8_t dat2[] = {0XE1, 0X40, 0X40, 0X01, 0X03, 0X0B, 0XD1, 0X01, 0X07, 0X55, 0X00, 0X54, 0X65, 0X73, 0X74, 0X20, 0X31, 0XFE};
    const uint8_t dat3[] = {0XE1, 0X40, 0X40, 0X01, 0X03, 0X12, 0XD1, 0X01, 0X0E, 0X55, 0X00, 0X54, 0X65, 0X73, 0X74, 0X20, 0X32, 0X20, 0X6C, 0X6F, 0X6E, 0X67, 0X65, 0X72, 0XFE};
    const uint8_t dat4[] = {0XE1, 0X40, 0X40, 0X01, 0X03, 0X2C, 0XD1, 0X01, 0X28, 0X55, 0X00, 0X54, 0X65, 0X73, 0X74, 0X20, 0X33, 0X20, 0X65, 0X76, 0X65, 0X6E, 0X20, 0X6C, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6F, 0X6E, 0X67, 0X65, 0X72, 0XFE};
    const uint8_t dat5[] = {0XE1, 0X40, 0X40, 0X01, 0X03, 0X24, 0XD1, 0X01, 0X20, 0X55, 0X02, 0X79, 0X6F, 0X75, 0X74, 0X75, 0X62, 0X65, 0X2E, 0X63, 0X6F, 0X6D, 0X2F, 0X77, 0X61, 0X74, 0X63, 0X68, 0X3F, 0X76, 0X3D, 0X64, 0X51, 0X77, 0X34, 0X77, 0X39, 0X57, 0X67, 0X58, 0X63, 0X51, 0XFE};
    switch (eepromData % 5)
    {
    case 0: // "i have been read"
    default:
        pData = &dat1[0];
        pData[25] = (eepromData % 10) + '0';
        pData[24] = ((eepromData / 10) % 10) + '0';
        pData[23] = ((eepromData / 100) % 10) + '0';
        break;
    case 1: // "Test 1"
        pData = &dat2[0];
        break;
    case 2: // "Test 2 longer"
        pData = &dat3[0];
        break;
    case 3: // "Test 3 even loooooooooooooooooooooonger"
        pData = &dat4[0];
        break;
    case 4: // "Rick roll"
        pData = &dat5[0];
        break;
    }

    uint8_t state = 0;
    uint8_t tries = 0;
    // write message to NRF device
    while (state != 3 && tries <= 5)
    {

        if ((PORTA.IN & PIN6_bm))
        {

            // pin is high go to next step
            if (state == 0)
            {
                if (TWI_sendBytesToRegister(SLAVE_ADDRESS_USER, 0, pData, pData[5]+7))
                {
                    state = 1;
                }
            }
            else
            {
                if (TWI_VerifyBytes(SLAVE_ADDRESS_USER, 0, pData, pData[5]+7))
                {
                    state = 3;
                    break;
                }
                else
                {
                    tries++;
                    state = 0;
                }
            }
        }
        harwareWait();
    }

    // increment EEprom
    eeprom_write_byte(EEpromIncAdr, eepromData);
    
    nightynighty(); // go to sleep

    return 0; // never reached
}

void nightynighty()
{
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu(); // Enter sleep mode
}

void nightyWakey()
{
    set_sleep_mode(SLEEP_MODE_STANDBY); // sleep mode standby to recover by timers
    sleep_enable();
    sei();           // Enable interrupts
    sleep_cpu();     // Enter sleep mode
    sleep_disable(); // Disable sleep mode after waking up
    // cli();                 // Disable interrupts
    TCB0.CTRLA &= ~TCB_ENABLE_bm; // Disable the timer
    TCB0.CNT = 0;                 // Reset the timer value in case of future use
}

// ISR for timer B
ISR(TCB0_INT_vect)
{
    TCB0.INTFLAGS = TCB_CAPT_bm; // Clear interrupt flag
    sleep_disable();             // Disable sleep mode after waking up
}

// Set the timer to generate an interrupt every 1000ms
void init_timer()
{
    TCB0.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;             // Clock source: CLK_PER / 2 (16.384 kHz)
    TCB0.CTRLB |= TCB_CNTMODE_INT_gc;                // Periodic interrupt mode
    TCB0.CCMP = 65530;                               // Set period for 100ms (16,384 Hz clock)
    TCB0.INTCTRL |= TCB_CAPT_bm;                     // Enable interrupt on capture/compare match
    TCB0.CTRLA |= (TCB_ENABLE_bm | TCB_RUNSTDBY_bm); // Enable TCB and allow run in standby
}

// harware wait function
void harwareWait()
{
    init_timer();
    nightyWakey();
}
