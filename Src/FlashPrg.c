/*********************************************************************
*            (c) 1995 - 2018 SEGGER Microcontroller GmbH             *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************
----------------------------------------------------------------------
File    : FlashPrg.c
Purpose : Implementation of RAMCode for ST STM32F205RC
--------  END-OF-HEADER  ---------------------------------------------
*/
//#include "main.h"
//#include "quadspi.h"
//#include "gpio.h"
#include "FlashOS.H"
#include "w25q64.h"


#define SUPPORT_BLANK_CHECK         (0)  // No separate blank check is required as the flash is memory mapped readable
#define SUPPORT_SEGGER_OPEN_READ    (1)
#define SUPPORT_SEGGER_OPEN_Program (1)
#define SUPPORT_SEGGER_OPEN_ERASE   (0)


#define PAGE_SIZE_SHIFT (8)             // 8 bytes page size

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
static volatile int _Dummy;

const U32 SEGGER_OPEN_FLASHLOADER_FLAGS = 0xFEED;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Init
*
*  Function description
*    Handles the initialization of the flash module.
*
*  Parameters
*    Addr: Flash base address
*    Freq: Clock frequency in Hz
*    Func: Specifies the action followed by Init() (e.g.: 1 - Erase, 2 - Program, 3 - Verify / Read)
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int Init(U32 Addr, U32 Freq, U32 Func) {
  U32 v,ID;
  (void)Addr;
  (void)Freq;
  (void)Func;
  
  Stm32_Clock_Init(200, 32, 1, 1);

//  LL_RCC_SetQSPIClockSource(LL_RCC_QSPI_CLKSOURCE_HCLK);

  //qspi_init();
  QSPI_Init();

  W25QXX_Init();

  ID = W25QXX_ReadId();

  QSPI_MMAP();

  //qspi_mmap();

  return 0;
}

/*********************************************************************
*
*       UnInit
*
*  Function description
*    Handles the de-initialization of the flash module.
*
*  Parameters
*    Func: Caller type (e.g.: 1 - Erase, 2 - Program, 3 - Verify)
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int UnInit(U32 Func) {
  U32 v;
  (void)Func;

  return 0;
}

/*********************************************************************
*
*       EraseSector
*
*  Function description
*    Erases one flash sector.
*
*  Parameters
*    SectorAddr: Absolute address of the sector to be erased
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int EraseSector(U32 SectorAddr) {

	SectorAddr -= 0x90000000;

//	qspi_init();

	//qspi_erase_sector(SectorAddr);
        W25QXX_EraseSector(SectorAddr);

	//qspi_mmap();
        QSPI_MMAP();
        

	return 0;
}

#if 1  // We do not support EraseChip() as it is much slower than erase the flash sector by sector
/*********************************************************************
*
*       EraseChip
*
*  Function description
*    Erases the entire flash
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int EraseChip(void) {
  return 0;
}
#endif

/*********************************************************************
*
*       ProgramPage
*
*  Function description
*    Programs one flash page.
*
*  Parameters
*    DestAddr: Destination address
*    NumBytes: Number of bytes to be programmed (always a multiple of program page size, defined in FlashDev.c)
*    pSrcBuff: Point to the source buffer
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int ProgramPage(U32 DestAddr, U32 NumBytes, U8 *pSrcBuff) {

	DestAddr -= 0x90000000;

	//qspi_write(DestAddr, pSrcBuff, NumBytes);
        W25QXX_Write(pSrcBuff, DestAddr, NumBytes);
	//qspi_mmap();
        QSPI_MMAP();

	return 0;
}

/*********************************************************************
*
*       BlankCheck
*
*  Function description
*    Checks if a memory region is blank
*
*  Parameters
*    Addr: Blank check start address
*    NumBytes: Number of bytes to be checked
*    BlankData: Pointer to the destination data
*
*  Return value 
*    0: O.K., blank
*    1: O.K., *not* blank
*    < 0: Error
*
*/
#if SUPPORT_BLANK_CHECK
int BlankCheck(U32 Addr, U32 NumBytes, U8 BlankData) {
  U8* pData;
  
  pData = (U8 *)Addr;
  do {
    if (*pData++ != BlankData) {
      return 1;
    }
  } while (--NumBytes);
  return 0;
}
#endif

/*********************************************************************
*
*       SEGGER_OPEN_Read
*
*  Function description
*    Reads a specified number of bytes into the provided buffer
*
*  Parameters
*    Addr: Start read address
*    NumBytes: Number of bytes to be read
*    pBuff: Pointer to the destination data
*
*  Return value 
*    >= 0: O.K., NumBytes read
*    <  0: Error
*
*/
#if SUPPORT_SEGGER_OPEN_READ
int SEGGER_OPEN_Read(U32 Addr, U32 NumBytes, U8 *pDestBuff) {
  int i;
  U8 *flash_addr = (U8 *)Addr;
  for (i = 0; i < NumBytes; i++) {
    *pDestBuff++ = *flash_addr++;
  }
  //
  // Read function
  // Add your code here...
  //
  return NumBytes;
}
#endif

/*********************************************************************
*
*       SEGGER_OPEN_Program
*
*  Function description
*    Programs a specified number of bytes into the target flash.
*    NumBytes is either FlashDevice.PageSize or a multiple of it.
*
*  Notes
*    (1) This function can rely on that at least FlashDevice.PageSize will be passed
*    (2) This function must be able to handle multiple of FlashDevice.PageSize
*
*  Parameters
*    Addr: Start read address
*    NumBytes: Number of bytes to be read
*    pBuff: Pointer to the destination data
*
*  Return value 
*    0 O.K.
*    1 Error
*
*/
#if SUPPORT_SEGGER_OPEN_Program
int SEGGER_OPEN_Program(U32 DestAddr, U32 NumBytes, U8 *pSrcBuff) {
  U32 NumPages;
  int r;

  NumPages = (NumBytes >> PAGE_SIZE_SHIFT);
  r = 0;
  do {
    r = ProgramPage(DestAddr, (1uL << PAGE_SIZE_SHIFT), pSrcBuff);
    if (r < 0) {
      return r;
    }
    DestAddr += (1uL << PAGE_SIZE_SHIFT);
    pSrcBuff += (1uL << PAGE_SIZE_SHIFT);
  } while (--NumPages);
  return r;
}
#endif

/*********************************************************************
*
*       SEGGER_OPEN_Erase
*
*  Function description
*    Erases one or more flash sectors
*
*  Notes
*    (1) This function can rely on that at least one sector will be passed
*    (2) This function must be able to handle multiple sectors at once
*    (3) This function can rely on that if sector size changes, 
*
*  Parameters
*    SectorAddr: Address of the start sector to be erased
*    SectorIndex: Index of the start sector to be erased
*    NumSectors: Number of sectors to be erased. At least 1 sector is passed.
*
*  Return value 
*    0 O.K.
*    1 Error
*
*/
#if SUPPORT_SEGGER_OPEN_ERASE
int SEGGER_OPEN_Erase(U32 SectorAddr, U32 SectorIndex, U32 NumSectors) {
  (void)SectorAddr;
  return 0;
}
#endif
  
