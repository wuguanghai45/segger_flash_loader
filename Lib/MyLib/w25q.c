#include "w25q.h"

static void w25q_page_write(uint8_t *data, uint32_t addr, uint16_t count);
static void w25q_wait(void);
static void w25q_write_en(void);

void w25q_init(void)
{
	qspi_init();
}

uint32_t w25q_read_jedec_id(void)
{
	uint8_t id[3];

	qspi_cmd_read(W25X_JedecDeviceID, id, 3);
	return ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | id[2];
}

uint16_t w25q_read_id(void)
{
	uint8_t id[2];

	qspi_cmd_addr_read(W25X_ManufactDeviceID, 0, id, 2);
	return ((uint16_t)id[0] << 8) | id[1];
}

void w25q_read(uint8_t *data, uint32_t addr, uint32_t count)
{
	qspi_cmd_addr_read(W25X_ReadData, addr, data, count);
}

void w25q_write(uint8_t *data, uint32_t addr, uint32_t count)
{
	while (count) {
		uint32_t chunk = count;

		if (chunk > W25Q_PageSize) {
			chunk = W25Q_PageSize;
		}
		w25q_page_write(data, addr, (uint16_t)chunk);
		count -= chunk;
		data += chunk;
		addr += chunk;
	}
}

void w25q_erase(uint32_t addr, enum w25q_erase_modes mode)
{
	w25q_write_en();

	if (mode == W25Q_ERASE_CHIP) {
		qspi_cmd_only((uint8_t)mode);
	} else {
		qspi_cmd_addr_only((uint8_t)mode, addr);
	}

	w25q_wait();
}

static void w25q_page_write(uint8_t *data, uint32_t addr, uint16_t count)
{
	w25q_write_en();

	if (count > W25Q_PageSize) {
		count = W25Q_PageSize;
	}

	if (count == W25Q_PageSize) {
		addr &= 0xFFFF00U;
	}

	qspi_cmd_addr_write(W25X_PageProgram, addr, data, count);
	w25q_wait();
}

static void w25q_write_en(void)
{
	uint8_t status;

	qspi_cmd_only(W25X_WriteEnable);

	do {
		qspi_cmd_addr_read(W25X_ReadStatusReg, 0, &status, 1);
	} while ((status & 0x02U) == 0U);
}

static void w25q_wait(void)
{
	uint8_t status;

	do {
		qspi_cmd_addr_read(W25X_ReadStatusReg, 0, &status, 1);
	} while (status & 0x01U);
}
