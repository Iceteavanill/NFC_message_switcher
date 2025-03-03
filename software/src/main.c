/*
This software was written by Fabian Steiner for the NFC_message_switcher Project.
It is provided AS IS with no liability for anything this software may or may not be the cause of.
For more information check the github readme.
This Project uses various AVR libraries
*/

#include <xc.h>
#include <stdint.h>
#include "st25dv04k.h"
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

// debug for more precise error recording
#define debugIsActive 0

// EEprom Data adresses
uint8_t *EEpromIncAdr = (uint8_t *)0x00;  // Adress for incremental acces counting (gets looped around after 255)
uint8_t *EEpromInitAdr = (uint8_t *)0x01; // Adress for storage of Initialisation status

#if debugIsActive
const uint8_t *EEpromErrAdr = (uint8_t *)0x01; // Adress to store Error States (for debugging)
uint8_t errorData = 0x00;                      // error that may have happened
ST25DV04K_status debugStateST25D = ST25DV04K_Undef;
/*
errorData :
    0x01 -> Not inited but EV state correct
    0x02 -> Error with I2C Communication
    0x04 -> after write of EH mode, correct stae was not set -> wtf?
*/
#endif

// functions
void nightynighty(); // func to go to sleep indefinitely -> after NFC tag has been written
void nightyWakey();  // function to go to sleep but also read Interrupt and wake up
void init_timer();   // init the timer for periodic interrupt

// fuse definition
FUSES = {
    .WDTCFG = 0x00,  // WDTCFG {PERIOD=OFF, WINDOW=OFF}
    .BODCFG = 0x10,  // BODCFG {SLEEP=DIS, ACTIVE=DIS, SAMPFREQ=125Hz, LVL=BODLEVEL0}
    .OSCCFG = 0x01,  // OSCCFG {FREQSEL=16MHZ, OSCLOCK=CLEAR}
    .TCD0CFG = 0x00, // TCD0CFG {CMPA=CLEAR, CMPB=CLEAR, CMPC=CLEAR, CMPD=CLEAR, CMPAEN=CLEAR, CMPBEN=CLEAR, CMPCEN=CLEAR, CMPDEN=CLEAR}
    .SYSCFG0 = 0xF7, // SYSCFG0 {EESAVE=SET, RSTPINCFG=UPDI, CRCSRC=NOCRC}
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

    st25dv04k_init(); // Init I2C connection

    // Set PA6 (GPO input) as input
    PORTA.DIRCLR = PIN6_bm;
    // enable pullup (is needed because open drain output)
    PORTA.PIN6CTRL |= PORT_PULLUPEN_bm;

    if (eeprom_read_byte(EEpromInitAdr) != 0x69) // check if the ST25DV04 device has been Inited
    {
        // Password for system access (default)
        uint8_t defaultpasswordSequence[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// debug mode -> more, and finer info
#if debugIsActive == 1
        uint8_t readdata = 0xFF; // generell read buffer for reading from the device

        // check if EH mode is set to 0 (forced after boot)
        debugStateST25D = st25dv04k_read(SLAVE_ADDRESS_SYSTEM, SysReg_EH_MODE, &readdata, 1);
        errorData |= (debugStateST25D == ST25DV04K_SUCCESS ? 0 : 0x02);

        if (readdata == 0) // maybe dont check in production code and just write :)
        {
            // EH mode is correctly set -> continue
            errorData |= 0x01;
        }
        else
        {
            // Provide Password to ST25DV device
            debugStateST25D = st25dv04k_write_page(SLAVE_ADDRESS_SYSTEM, SysReg_PasswordAdr, &defaultpasswordSequence[0], 17);
            errorData |= (debugStateST25D == ST25DV04K_SUCCESS ? 0 : 0x02);

            // now that a Secure session is open -> write EH mode
            debugStateST25D = st25dv04k_write_page(SLAVE_ADDRESS_SYSTEM, SysReg_EH_MODE, (uint8_t *)0, 1);
            errorData |= (debugStateST25D == ST25DV04K_SUCCESS ? 0 : 0x02);

            // double check correct state set
            debugStateST25D = st25dv04k_read(SLAVE_ADDRESS_SYSTEM, SysReg_EH_MODE, &readdata, 1);
            errorData |= (debugStateST25D == ST25DV04K_SUCCESS ? 0 : 0x02);
            errorData |= (readdata == 0 ? 0 : 0x04);
        }
        // check if GPO output ist set to RF_ACTIVITY_EN (with enable bit)
        debugStateST25D = st25dv04k_read(SLAVE_ADDRESS_SYSTEM, SysReg_GPO, &readdata, 1);
        errorData |= (readdata == 0 ? 0 : 0x02);

        if (readdata == 0x82) // check for RF_ACTIVITY_EN and enable bit -> b10000010
        {
            // GPO mode is correctly set -> continue
            errorData |= 0x01;
        }
        else
        {
            // dont provide password because if one was not written, both were probabbly not written

            // now that a Secure session is open -> write EH mode
            debugStateST25D = st25dv04k_write_page(SLAVE_ADDRESS_SYSTEM, SysReg_EH_MODE, (uint8_t *)0, 1);
            errorData |= (debugStateST25D == ST25DV04K_SUCCESS ? 0 : 0x02);

            // double check correct state set
            debugStateST25D = st25dv04k_read(SLAVE_ADDRESS_SYSTEM, SysReg_GPO, &readdata, 1);
            errorData |= (debugStateST25D == ST25DV04K_SUCCESS ? 0 : 0x02);
            errorData |= (readdata == 0x82 ? 0 : 0x04);
        }
#else // no debugging

        // Provide Password to ST25DV device
        st25dv04k_write_page(SLAVE_ADDRESS_SYSTEM, SysReg_PasswordAdr, &defaultpasswordSequence[0], 17);
        // now that a Secure session is open -> write EH mode
        st25dv04k_write_page(SLAVE_ADDRESS_SYSTEM, SysReg_EH_MODE, (uint8_t *)0, 1);
        // write GPO mode
        st25dv04k_write_page(SLAVE_ADDRESS_SYSTEM, SysReg_EH_MODE, (uint8_t *)0x82, 1);
#endif

        eeprom_write_byte(EEpromInitAdr, 0x69); // ST25dv was initialized -> mark as initialized
        nightynighty();
    }

    uint8_t *pData;      // datapointer
    uint8_t datalen = 0; // length of pointer

    // read eeprom inc adress
    uint8_t eepromData = eeprom_read_byte(EEpromIncAdr);
    if (eepromData == 255)
    {
        eepromData = 0;
    }
    else
    {
        eepromData++;
    }

    // CHECK DATALENGHT AND SET IT CORRECTY!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //testdata -> edit for usecase
    uint8_t dat1[] = {0XE1, 0X40, 0X40, 0X00, 0X03, 0X1A, 0XD1, 0X01, 0X16, 0X54, 0X02, 0X65, 0X6E, 0X49, 0X20, 0X77, 0X61, 0X73, 0X20, 0X72, 0X75, 0X6E, 0X20, 0X20, 0X20, 0X31, 0X20, 0X74, 0X69, 0X6D, 0X65, 0X73, 0XFE};
    uint8_t dat2[] = {0XE1, 0X40, 0X40, 0X00, 0X03, 0X24, 0XD1, 0X01, 0X20, 0X55, 0X02, 0X79, 0X6F, 0X75, 0X74, 0X75, 0X62, 0X65, 0X2E, 0X63, 0X6F, 0X6D, 0X2F, 0X77, 0X61, 0X74, 0X63, 0X68, 0X3F, 0X76, 0X3D, 0X64, 0X51, 0X77, 0X34, 0X77, 0X39, 0X57, 0X67, 0X58, 0X63, 0X51, 0XFE};
    uint8_t dat3[] = {0XE1, 0X40, 0X40, 0X00, 0X03, 0X1F, 0XD1, 0X01, 0X1B, 0X55, 0X04, 0X73, 0X74, 0X75, 0X64, 0X65, 0X6E, 0X74, 0X65, 0X6E, 0X70, 0X6F, 0X72, 0X74, 0X61, 0X6C, 0X2E, 0X63, 0X68, 0X2F, 0X7A, 0X69, 0X74, 0X61, 0X74, 0X65, 0X2F, 0XFE};
    uint8_t dat4[] = {0XE1, 0X40, 0X40, 0X00, 0X03, 0X1B, 0XD1, 0X01, 0X17, 0X55, 0X00, 0X67, 0X65, 0X6F, 0X3A, 0X34, 0X37, 0X2E, 0X32, 0X32, 0X33, 0X39, 0X33, 0X37, 0X2C, 0X38, 0X2E, 0X38, 0X31, 0X37, 0X33, 0X31, 0X32, 0XFE};

    switch (eepromData % 4)
    {
    case 0: // "i have been read"
    default:
        pData = &dat1[0];
        pData[25] = (eepromData % 10) + '0';
        pData[24] = ((eepromData / 10) % 10) + '0';
        pData[23] = ((eepromData / 100) % 10) + '0';
        datalen = 33;
        break;
    case 1: // "rickroll"
        pData = &dat2[0];
        datalen = 43;
        break;
    case 2: // "studenteportal"
        pData = &dat3[0];
        datalen = 38;
        break;
    case 3: // "Location"
        pData = &dat4[0];
        datalen = 34;
        break;
    }

    // write message to NRF device
    ST25DV04K_status i2cSuccess = ST25DV04K_Undef;
    while (i2cSuccess != ST25DV04K_SUCCESS)
    {
        if (!(PORTA.IN & PIN6_bm))
        {
            // port is low, wait for timer interrupt.
            init_timer();
            nightyWakey();
        }

        i2cSuccess = st25dv04k_write_page(SLAVE_ADDRESS_USER, 0, pData, datalen);
        if (i2cSuccess == ST25DV04K_SUCCESS)
        {
            break;
        }
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
    sei();                 // Enable interrupts
    sleep_cpu();           // Enter sleep mode
    sleep_disable();       // Disable sleep mode after waking up
    cli();                 // Disable interrupts
    TCA0.SINGLE.CTRLA = 0; // Disable the timer
    TCA0.SINGLE.CNT = 0;   // Reset the timer value in case of future use
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
    TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc;              // Clock source: CLK_PER / 2 (16.384 kHz)
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;                 // Periodic interrupt mode
    TCB0.CCMP = 16400;                               // Set period for 1000ms (16,384 Hz clock)
    TCB0.INTCTRL = TCB_CAPT_bm;                      // Enable interrupt on capture/compare match
    TCB0.CTRLA |= (TCB_ENABLE_bm | TCB_RUNSTDBY_bm); // Enable TCB and allow run in standby
}
