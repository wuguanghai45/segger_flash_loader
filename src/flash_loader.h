#ifndef SEG_FL_H
#define SEG_FL_H
#include "FlashOS.h"

#define PrgCode  __attribute__ ((section ("PrgCode"), __used__))
#define DevDescr __attribute__ ((section ("DevDscr")))

int SEGGER_FL_Prepare(U32 PreparePara0, U32 PreparePara1, U32 PreparePara2);
int SEGGER_FL_Restore(U32 RestorePara0, U32 RestorePara1, U32 RestorePara2);
int SEGGER_FL_Program(U32 DestAddr, U32 NumBytes, U8 *pSrcBuff);
int SEGGER_FL_Erase(U32 SectorAddr, U32 SectorIndex, U32 NumSectors);
int SEGGER_FL_EraseChip(void);
int SEGGER_FL_CheckBlank(U32 Addr, U32 NumBytes, U8 BlankValue);
int SEGGER_FL_Read(U32 Addr, U32 NumBytes, U8 *pDestBuff);

#endif