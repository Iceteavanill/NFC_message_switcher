#ifndef I2C_HOST_H
#define I2C_HOST_H

#include <stdint.h>
#include <avr/io.h>

typedef enum TWI_host_status
{
  TWI_INIT = 0,
  TWI_READY,
  TWI_ARBITRATION_LOST,
  TWI_NO_ACKNOWLEDGEMENT,
  TWI_BUS_ERROR,
  TWI_SUCCESS
} TWI_host_status;

typedef enum TWI_host_direction
{
  TWI_WRITE = 0,
  TWI_READ = 1
} TWI_host_direction;

TWI_host_status i2c_host_init();
TWI_host_status i2c_host_get_write_status(void);
TWI_host_status i2c_host_get_read_status(void);
TWI_host_status i2c_host_init_transmission(uint8_t address, TWI_host_direction direction);
TWI_host_status i2c_host_write_byte(uint8_t data);
TWI_host_status i2c_host_read_byte(uint8_t *pData);
void i2c_host_acknowledge();
void i2c_host_no_acknowledge();
void i2c_host_no_acknowledge_stop();
void i2c_host_stop();
void i2c_host_abort();
void i2c_handle_error(TWI_host_status status);

#endif // I2C_HOST_H
/*----------------------------------------------------------------------------*/
