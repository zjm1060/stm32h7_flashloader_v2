/*********************************************************************
*            (c) 1995 - 2018 SEGGER Microcontroller GmbH             *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************
----------------------------------------------------------------------
File    : FlashDev.c
Purpose : Flash device description for ST STM32F205RC
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "FlashOS.H"

struct FlashDevice const FlashDevice __attribute__ ((section ("DevDscr"))) =  {
  ALGO_VERSION,             // Algo version
  "STM32H7_W25Q16", // Flash device name
  5,                   // Flash device type
  0x90000000,               // Flash base address
  0x00200000,               // Total flash device size in Bytes (2 MB)
  256,                        // Page Size (number of bytes that will be passed to ProgramPage(). MinAlig is 8 byte
  0,                        // Reserved, should be 0
  0xFF,                     // Flash erased value
  1000,                      // Program page timeout in ms
  6000,                     // Erase sector timeout in ms
  //
  // Flash sector layout definition
  //
  0x00001000, 0x00000000,   // 4 *  16 KB =  64 KB
  0xFFFFFFFF, 0xFFFFFFFF    // Indicates the end of the flash sector layout. Must be present.
};