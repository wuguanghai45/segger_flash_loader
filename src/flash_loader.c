#include "flash_loader.h"
#include "w25q.h"
#include "qspi.h"
#include "rcc.h"
#include "log.h"

#define CHECK_BLANK_CHUNK 256U

struct FlashDevice const FlashDevice DevDescr = {
	ALGO_VERSION,
	"L431 W25Q16",
	ONCHIP,
	0x90000000,
	0x200000,
	256,
	0,
	0xFF,
	100,
	40000,
	{
		{0x00001000, 0x00000000},
		{0xFFFFFFFF, 0xFFFFFFFF}
	}
};

int PrgCode SEGGER_FL_Prepare(U32 PreparePara0, U32 PreparePara1, U32 PreparePara2)
{
	uint32_t jedec;

	(void)PreparePara0;
	(void)PreparePara1;
	(void)PreparePara2;

	dwt_init();
	RCC_init();

	DEBUG("=>Prepare call");

	w25q_init();
	jedec = w25q_read_jedec_id();
	DEBUG("JEDEC ID 0x%06X", jedec);

	if (((jedec >> 16) & 0xFFU) == W25Q_JEDEC_MANUF &&
	    (((jedec >> 8) & 0xFFU) == W25Q_JEDEC_MEMTYPE) &&
	    ((jedec & 0xFFU) == W25Q_JEDEC_CAPACITY)) {
		qspi_enter_memory_mapped();
		return 0;
	}

	return -1;
}

int PrgCode SEGGER_FL_Restore(U32 RestorePara0, U32 RestorePara1, U32 RestorePara2)
{
	(void)RestorePara0;
	(void)RestorePara1;
	(void)RestorePara2;

	DEBUG("=>Restore call");
	qspi_exit_memory_mapped();
	return 0;
}

int PrgCode SEGGER_FL_Program(U32 DestAddr, U32 NumBytes, U8 *pSrcBuff)
{
	uint32_t addr = 0;

	DEBUG("=>Program call addr:0x%0X %d bytes", DestAddr, NumBytes);

	if (DestAddr >= FlashDevice.BaseAddr) {
		addr = DestAddr - FlashDevice.BaseAddr;
	} else {
		addr = DestAddr;
	}

	w25q_write(pSrcBuff, addr, NumBytes);
	return 0;
}

int PrgCode SEGGER_FL_Erase(U32 SectorAddr, U32 SectorIndex, U32 NumSectors)
{
	uint32_t addr = SectorAddr;
	uint32_t sector_size = FlashDevice.SectorInfo[0].SectorSize;

	(void)SectorIndex;

	DEBUG("=>Erase call addr:0x%0X index:%d sectors:%d", SectorAddr, SectorIndex, NumSectors);

	if (addr >= FlashDevice.BaseAddr) {
		addr -= FlashDevice.BaseAddr;
	}

	if (sector_size != 0U) {
		addr &= ~(sector_size - 1U);
	}

	for (uint32_t n = 0; n < NumSectors; n++) {
		w25q_erase(addr, W25Q_ERASE_SECTOR);
		addr += sector_size;
	}
	return 0;
}

int PrgCode SEGGER_FL_CheckBlank(U32 Addr, U32 NumBytes, U8 BlankValue)
{
	uint32_t addr;
	uint8_t buf[CHECK_BLANK_CHUNK];

	if (Addr >= FlashDevice.BaseAddr) {
		addr = Addr - FlashDevice.BaseAddr;
	} else {
		addr = Addr;
	}

	while (NumBytes > 0U) {
		uint32_t chunk = NumBytes;

		if (chunk > CHECK_BLANK_CHUNK) {
			chunk = CHECK_BLANK_CHUNK;
		}

		w25q_read(buf, addr, chunk);

		for (uint32_t i = 0; i < chunk; i++) {
			if (buf[i] != BlankValue) {
				return 1;
			}
		}

		addr += chunk;
		NumBytes -= chunk;
	}

	return 0;
}

int PrgCode SEGGER_FL_EraseChip(void)
{
	DEBUG("=>Erase chip call");
	w25q_erase(0, W25Q_ERASE_CHIP);
	return 0;
}

int PrgCode SEGGER_FL_Read(U32 Addr, U32 NumBytes, U8 *pDestBuff)
{
	uint32_t addr = 0;

	DEBUG("=>Read call addr:0x%0X %d bytes", Addr, NumBytes);

	if (Addr >= FlashDevice.BaseAddr) {
		addr = Addr - FlashDevice.BaseAddr;
	}

	w25q_read(pDestBuff, addr, NumBytes);
	return (int)NumBytes;
}
