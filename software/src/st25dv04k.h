#ifndef ST25DV04K_H
#define ST25DV04K_H

#include <stdint.h>
#include "i2c_host.h"

typedef enum ST25DV04K_status
{
  ST25DV04K_UNDEF = 10,
  ST25DV04K_ERROR_I2C = -1,
  ST25DV04K_SUCCESS = 0,
  ST25DV04K_VERYFYFAIL = 8
} ST25DV04K_status;

/*
0101 0X11 MSB 0 due to 7 bit adresses
0101 -> fixed device ardess
0X11 ->  X determines if accessing system area or user memory
X = 0, access to user memory, Dynamic registers or Mailbox.
X = 1, access to system area.

 */
typedef enum ST25DV04K_slave_address
{ // 0101
  SLAVE_ADDRESS_SYSTEM = 0x57,
  SLAVE_ADDRESS_USER = 0x53
} ST25DV04K_slave_address;

enum RegularInterrestingAdresses
{
  RF_MNGT_Dyn = 0x2003,
  I2C_SSO_Dyn = 0x2004

};

enum SystemRegisterOffsets
{
  SysReg_GPO = 0x00,
  SysReg_IT_TIME = 0x01,
  SysReg_EH_MODE = 0x02,
  SysReg_RF_MNGT = 0x03,
  SysReg_RFA1SS = 0x04,
  SysReg_ENDA1 = 0x05,
  SysReg_RFA2SS = 0x06,
  SysReg_ENDA2 = 0x07,
  SysReg_PasswordAdr = 0x0900
};
/*
00h RW (1) GPO Enable/disable ITs on GPO E2=1 0000h RW (2)
01h RW(1) IT_TIME Interruption pulse duration E2=1 0001h RW(2)
02h RW(1) EH_MODE Energy Harvesting default strategy after Power ON E2=1 0002h RW(2)
03h RW(1) RF_MNGT RF interface state after Power ON E2=1 0003h RW(2)
04h RW(1) RFA1SS Area1 RF access protection E2=1 0004h RW(2)
05h RW(1) ENDA1 Area 1 ending point E2=1 0005h RW(2)
06h RW(1) RFA2SS Area2 RF access protection E2=1 0006h RW(2)
07h RW(1) ENDA2 Area 2 ending point E2=1 0007h RW(2)
08h RW(1) RFA3SS Area3 RF access protection E2=1 0008h RW(2)
09h RW(1) ENDA3 Area 3 ending point E2=1 0009h RW(2)
0Ah RW(1) RFA4SS Area4 RF access protection E2=1 000Ah RW(2)
No access I2CSS Area 1 to 4 I2C access protection E2=1 000Bh RW(2)
N/A RW (3) (4) LOCK_CCFILE Blocks 0 and 1 RF Write protection E2=1 000Ch RW(2)
0Dh RW(1) MB_MODE Fast transfer mode state after power ON E2=1 000Dh RW(2)
0Eh RW(1) MB_WDG Maximum time before the message is automatically
released E2=1 000Eh RW(2)
0Fh RW(1) LOCK_CFG Protect RF Write to system configuration registers E2=1 000Fh RW(2)
N/A WO (5) LOCK_DSFID DSFID lock status E2=1 0010h RO
NA WO (6) LOCK_AFI AFI lock status E2=1 0011h RO
N/A RW(5) DSFID DSFID value E2=1 0012h RO
N/A RW(6) AFI AFI value E2=1 0013h RO
N/A RO MEM_SIZE Memory size value in blocks, 2 bytes E2=1 0014h to 0015h RO
RO BLK_SIZE Block size value in bytes E2=1 0016h RO
N/A RO IC_REF IC reference value E2=1 0017h RO
NA RO UID Unique identifier, 8 bytes

*/

ST25DV04K_status st25dv04k_init();
ST25DV04K_status st25dv04k_write_page(ST25DV04K_slave_address slaveAddress, uint16_t address, uint8_t *pData, uint8_t len);
ST25DV04K_status st25dv04k_read_page(ST25DV04K_slave_address slaveAddress, uint16_t address, uint8_t *pData, uint8_t len);
ST25DV04K_status st25dv04k_write_verify(ST25DV04K_slave_address slaveAddress, uint16_t address, uint8_t *pData, uint8_t len);

#endif // ST25DV04K_H
