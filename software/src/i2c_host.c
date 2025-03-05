#include "i2c_host.h"

void reg_write(uint8_t* reg, uint8_t mask, uint8_t value)
{
  *reg &= ~mask;
  *reg |= value;
}

/*----------------------------------------------------------------------------*/
TWI_host_status i2c_host_init()
{
  // Only fast mode plus is being supported yet
  
  // disable interrupts
  reg_write(&TWI0.MCTRLA, TWI_RIEN_bm, (0 << TWI_RIEN_bp));
  reg_write(&TWI0.MCTRLA, TWI_WIEN_bm, (0 << TWI_WIEN_bp));

  // SDA setup time is four cycles
  reg_write(&TWI0.CTRLA, TWI_SDASETUP_bm, TWI_SDASETUP_4CYC_gc);
  // Hold time off (SMBus feature)
  reg_write(&TWI0.CTRLA, TWI_SDAHOLD_gm, TWI_SDAHOLD_OFF_gc);
  // Fast mode plus
  //TWI0.CTRLA = 0x2;
  //TWI0.CTRLA &= ~TWI_FMPEN_bm;
  //TWI0.CTRLA |= (1<<TWI_FMPEN_bp);
  reg_write(&TWI0.CTRLA, TWI_FMPEN_bm, (1<< TWI_FMPEN_bp));
  // TWI is halted in Break Debug mode and ignores events
  reg_write(&TWI0.DBGCTRL, TWI_DBGRUN_bm, (0 << TWI_DBGRUN_bp));

  // disable quick command enable (SMBus feature)
  reg_write(&TWI0.MCTRLA, TWI_QCEN_bm, (0 << TWI_QCEN_bp));
  // Bus timeout Disabled (SMBus feature)
  reg_write(&TWI0.MCTRLA, TWI_TIMEOUT_gm, TWI_TIMEOUT_DISABLED_gc);
  // Smart mode disable
  reg_write(&TWI0.MCTRLA, TWI_SMEN_bm, (0 << TWI_SMEN_bp));
  // set baud rate in fast mode plus
  // f_CLK_PER = 20 MHz
  // f_SCL = 1 MHz
  // tR = 120ns
  // tOF = 120ns
  // BAUD = 20Mhz*(500ns+120ns)-5 = 12.4-5 = 7.4 -> 8
  // tLow = (8+5)/20e6-120ns = 530ns
  TWI0.MBAUD = 8;

  // TWI disabled as client
  reg_write(&TWI0.SCTRLA, TWI_ENABLE_bm, (0 << TWI_ENABLE_bp));    
  // TWI enabled as host
  reg_write(&TWI0.MCTRLA, TWI_ENABLE_bm, (1 << TWI_ENABLE_bp));    
  // Busstate force to IDLE
  reg_write(&TWI0.MSTATUS, TWI_BUSSTATE_gm, TWI_BUSSTATE_IDLE_gc);
  // enable interrupts
  reg_write(&TWI0.MCTRLA, TWI_RIEN_bm, (1 << TWI_RIEN_bp));
  reg_write(&TWI0.MCTRLA, TWI_WIEN_bm, (1 << TWI_WIEN_bp));
 
  return TWI_SUCCESS;
}

/*----------------------------------------------------------------------------*/
TWI_host_status i2c_host_get_write_status(void)
{  
  TWI_host_status state = TWI_INIT;
  do{
    // read or write interrupt 
    if((TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm)) != 0x00)
    {
      if ((TWI0.MSTATUS & TWI_RXACK_bm) == 0x00)
      {
        state = TWI_READY;
      }
      else
      {
        state = TWI_NO_ACKNOWLEDGEMENT;
      }
    }
    else if((TWI0.MSTATUS & TWI_ARBLOST_bm) != 0x00)
    {
      state = TWI_ARBITRATION_LOST;
    }
    else if((TWI0.MSTATUS & TWI_BUSERR_bm) != 0x00)
    {
      state = TWI_BUS_ERROR;
    }    
  }while(state == TWI_INIT);

  return state;
}

/*----------------------------------------------------------------------------*/
TWI_host_status i2c_host_get_read_status(void)
{  
  TWI_host_status state = TWI_INIT;
  do{
    // read or write interrupt 
    if((TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm)) != 0x00)
    {
      state = TWI_READY;
    }
    else if((TWI0.MSTATUS & TWI_ARBLOST_bm) != 0x00)
    {
      state = TWI_ARBITRATION_LOST;
    }
    else if((TWI0.MSTATUS & TWI_BUSERR_bm) != 0x00)
    {
      state = TWI_BUS_ERROR;
    }    
  }while(state == TWI_INIT);

  return state;
}
  
/*----------------------------------------------------------------------------*/
TWI_host_status i2c_host_init_transmission(uint8_t address, TWI_host_direction direction)
{
  TWI_host_status status;
    TWI0.MADDR = (address << 1) | direction;
    status = i2c_host_get_write_status();
  return status;
}

/*----------------------------------------------------------------------------*/
TWI_host_status i2c_host_write_byte(uint8_t data)
{
  TWI0.MDATA = data;
  return i2c_host_get_write_status();
}

/*----------------------------------------------------------------------------*/
TWI_host_status i2c_host_read_byte(uint8_t* pData)
{
  TWI_host_status status;
  status = i2c_host_get_read_status();      
  *pData = TWI0.MDATA;
  return status;
}
  
/*----------------------------------------------------------------------------*/
void i2c_host_acknowledge()
{
  TWI0.MCTRLB = TWI_ACKACT_ACK_gc | TWI_MCMD_RECVTRANS_gc;
}

/*----------------------------------------------------------------------------*/
void i2c_host_no_acknowledge()
{
  TWI0.MCTRLB |= TWI_ACKACT_NACK_gc | TWI_MCMD_RECVTRANS_gc;
}

/*----------------------------------------------------------------------------*/
void i2c_host_stop()
{
  // Issue Stop condition
  TWI0.MCTRLB |= TWI_MCMD_STOP_gc;
}

/*----------------------------------------------------------------------------*/
void i2c_host_no_acknowledge_stop()
{
  // Issue Stop condition
  TWI0.MCTRLB |= TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
}
/*----------------------------------------------------------------------------*/
void i2c_host_abort()
{
  // aborts transmission
  reg_write(&TWI0.MCTRLB, TWI_FLUSH_bm, (1 << TWI_FLUSH_bp));
  uint8_t abortDone = 0;
  while(abortDone == 0)
  {
    if((TWI0.MSTATUS & TWI_BUSSTATE_gm) == TWI_BUSSTATE_IDLE_gc)
    {
      abortDone = 1;
    }
  }
}

/*----------------------------------------------------------------------------*/
void i2c_handle_error(TWI_host_status status)
{
  if(status == TWI_ARBITRATION_LOST)
  {
    i2c_host_abort();
  }
  else if(status == TWI_NO_ACKNOWLEDGEMENT)
  {
    i2c_host_stop();
  }
  else if(status == TWI_BUS_ERROR)
  {
    i2c_host_abort();
  }
}
