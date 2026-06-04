#ifndef W25Q_H
#define W25Q_H

#include "qspi.h"

#define W25X_WriteEnable        0x06
#define W25X_WriteDisable       0x04
#define W25X_ReadStatusReg      0x05
#define W25X_WriteStatusReg     0x01
#define W25X_ReadData           0x03
#define W25X_FastReadData       0x0B
#define W25X_PageProgram        0x02
#define W25X_BlockErase32       0x52
#define W25X_BlockErase64       0xD8
#define W25X_SectorErase        0x20
#define W25X_ChipErase          0xC7
#define W25X_JedecDeviceID      0x9F
#define W25X_ManufactDeviceID   0x90

#define W25Q_SectorSize         4096
#define W25Q_PageSize           256

#define W25Q_JEDEC_MANUF        0xEF
#define W25Q_JEDEC_MEMTYPE      0x40
#define W25Q_JEDEC_CAPACITY     0x15

enum w25q_erase_modes {
	W25Q_ERASE_SECTOR = W25X_SectorErase,
	W25Q_ERASE_BLOCK32 = W25X_BlockErase32,
	W25Q_ERASE_BLOCK64 = W25X_BlockErase64,
	W25Q_ERASE_CHIP = W25X_ChipErase
};

void w25q_init(void);
uint32_t w25q_read_jedec_id(void);
uint16_t w25q_read_id(void);
void w25q_erase(uint32_t addr, enum w25q_erase_modes mode);
void w25q_read(uint8_t *data, uint32_t addr, uint32_t count);
void w25q_write(uint8_t *data, uint32_t addr, uint32_t count);

#endif
