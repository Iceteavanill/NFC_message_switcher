#include "st25dv04k.h"

ST25DV04K_status st25dv04k_init()
{
  i2c_host_init();
  return ST25DV04K_SUCCESS;
}

/*----------------------------------------------------------------------------*/
ST25DV04K_status st25dv04k_write_page(ST25DV04K_slave_address slaveAddress, uint16_t address, uint8_t *pData, uint8_t len)
{
  TWI_host_status twiStatus;

  twiStatus = i2c_host_init_transmission(slaveAddress, TWI_WRITE);
  if (twiStatus != TWI_READY)
  {
    return ST25DV04K_ERROR_I2C;
  }

  // writes twice because adress is 16 bit
  twiStatus = i2c_host_write_byte((uint8_t)(address >> 8));
  if (twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }
  twiStatus = i2c_host_write_byte((uint8_t)address);
  if (twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }

  for (uint8_t i = 0; i < len; ++i)
  {
    twiStatus = i2c_host_write_byte(pData[i]);
    if (twiStatus != TWI_READY)
    {
      i2c_handle_error(twiStatus);
      return ST25DV04K_ERROR_I2C;
    }
  }
  i2c_host_stop();

  return ST25DV04K_SUCCESS;
}

/*----------------------------------------------------------------------------*/
ST25DV04K_status st25dv04k_read_page(ST25DV04K_slave_address slaveAddress, uint16_t address, uint8_t *pData, uint8_t len)
{
  TWI_host_status twiStatus;

  twiStatus = i2c_host_init_transmission(slaveAddress, TWI_WRITE);
  if (twiStatus != TWI_READY)
  {
    return ST25DV04K_ERROR_I2C;
  }
  // writes twice because adress is 16 bit
  twiStatus = i2c_host_write_byte((uint8_t)(address >> 8));
  if (twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }
  twiStatus = i2c_host_write_byte((uint8_t)address);
  if (twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }
  twiStatus = i2c_host_init_transmission(slaveAddress, TWI_READ);
  if (twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }

  for (uint8_t i = 0; i < len; i++)
  {
    twiStatus = i2c_host_read_byte(&(pData[i]));
    if (twiStatus != TWI_READY)
    {
      i2c_handle_error(twiStatus);
      return ST25DV04K_ERROR_I2C;
    }
    if (i == len - 1)
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

/*----------------------------------------------------------------------------*/
ST25DV04K_status st25dv04k_write_verify(ST25DV04K_slave_address slaveAddress, uint16_t address, uint8_t *pData, uint8_t len)
{
  ST25DV04K_status writeStatus = st25dv04k_write_page(slaveAddress, address, pData, len);

  if (writeStatus != ST25DV04K_SUCCESS)
  {
    return writeStatus;
  }

  TWI_host_status twiStatus;

  twiStatus = i2c_host_init_transmission(slaveAddress, TWI_WRITE);
  if (twiStatus != TWI_READY)
  {
    return ST25DV04K_ERROR_I2C;
  }
  // writes twice because adress is 16 bit
  twiStatus = i2c_host_write_byte((uint8_t)(address >> 8));
  if (twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }
  twiStatus = i2c_host_write_byte((uint8_t)address);
  if (twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }
  twiStatus = i2c_host_init_transmission(slaveAddress, TWI_READ);
  if (twiStatus != TWI_READY)
  {
    i2c_handle_error(twiStatus);
    return ST25DV04K_ERROR_I2C;
  }

  uint8_t datarecieve; // temporary buffer for reading operation
  uint8_t successCheck = 0;
  for (uint8_t i = 0; i < len; i++) // check for end of message
  {
    twiStatus = i2c_host_read_byte(&(datarecieve));
    if (twiStatus != TWI_READY)
    {
      i2c_handle_error(twiStatus);
      return ST25DV04K_ERROR_I2C;
    }
    if (i == len - 1)
    {
      i2c_host_no_acknowledge_stop();
    }
    else if (datarecieve != pData[i])
    {
      i2c_host_no_acknowledge_stop();
      successCheck = 1;
    }
    else
    {
      i2c_host_acknowledge();
    }
  }
  if (successCheck)
  {
    return ST25DV04K_VERYFYFAIL; // corupt data was detected (either corrupted read or write!)
  }
  else
  {
    return ST25DV04K_SUCCESS;
  }
}
