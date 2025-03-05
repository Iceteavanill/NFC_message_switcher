#include "st25dv04k.h"

ST25DV04K_status st25dv04k_init()
{
  i2c_host_init();
  return ST25DV04K_SUCCESS;
}

/*----------------------------------------------------------------------------*/
ST25DV04K_status st25dv04k_write_page(ST25DV04K_slave_address slaveAddress, uint16_t address, uint8_t* pData, uint8_t len)
{
  TWI_host_status twiStatus;

  twiStatus = i2c_host_init_transmission(slaveAddress, TWI_WRITE);
  if(twiStatus != TWI_READY)
  {
    return ST25DV04K_ERROR_I2C;
  }
  
  twiStatus = i2c_host_write_byte((uint8_t)(address>>8));
  if(twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }
  twiStatus = i2c_host_write_byte((uint8_t)address);
  if(twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }
  
  for(uint8_t i=0;i<len;++i)
  {
    twiStatus = i2c_host_write_byte(pData[i]);
    if(twiStatus != TWI_READY)
    {
      i2c_handle_error(twiStatus);
      return ST25DV04K_ERROR_I2C;
    }
  }
  i2c_host_stop();

  return ST25DV04K_SUCCESS;
  
}

/*----------------------------------------------------------------------------*/
ST25DV04K_status st25dv04k_read(ST25DV04K_slave_address slaveAddress, uint16_t address, uint8_t* pData, uint8_t len)
{
  TWI_host_status twiStatus;

  twiStatus = i2c_host_init_transmission(slaveAddress, TWI_WRITE);
  if(twiStatus != TWI_READY)
  {
    return ST25DV04K_ERROR_I2C;
  }
  twiStatus = i2c_host_write_byte((uint8_t)(address>>8));
  if(twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }
  twiStatus = i2c_host_write_byte((uint8_t)address);
  if(twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }
  twiStatus = i2c_host_init_transmission(slaveAddress, TWI_READ);
  if(twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }

  for(uint8_t i=0;i<len;i++)
  {
    twiStatus = i2c_host_read_byte(&(pData[i]));
    if(i==len-1)
    {
      i2c_host_no_acknowledge_stop();
    }
    else
    {
      i2c_host_acknowledge();
    }
  }
  return ST25DV04K_SUCCESS;
  
}
