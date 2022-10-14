#ifndef _W25Q64JV_H
#define _W25Q64JV_H

#include "qspi.h"

#define FLASH_ID                              0XEF4017

#define FLASH_OK                          ((uint8_t)0x00)
#define FLASH_ERROR                       ((uint8_t)0x01)
#define FLASH_BUSY                        ((uint8_t)0x02)
#define FLASH_NOT_SUPPORTED               ((uint8_t)0x04)
#define FLASH_SUSPENDED                   ((uint8_t)0x08)

#define RESET_ENABLE_CMD                      0x66
#define RESET_MEMORY_CMD                      0x99

#define READ_ID_CMD                           0x90
#define DUAL_READ_ID_CMD                      0x92
#define QUAD_READ_ID_CMD                      0x94
#define READ_JEDEC_ID_CMD                     0x9F

#define READ_CMD                              0x03
#define FAST_READ_CMD                         0x0B
#define DUAL_OUT_FAST_READ_CMD                0x3B
#define DUAL_INOUT_FAST_READ_CMD              0xBB
#define QUAD_OUT_FAST_READ_CMD                0x6B
#define QUAD_INOUT_FAST_READ_CMD              0xEB

#define WRITE_ENABLE_CMD                      0x06
#define WRITE_DISABLE_CMD                     0x04

#define READ_STATUS_REG1_CMD                  0x05
#define READ_STATUS_REG2_CMD                  0x35
#define READ_STATUS_REG3_CMD                  0x15

#define WRITE_STATUS_REG1_CMD                 0x01
#define WRITE_STATUS_REG2_CMD                 0x31
#define WRITE_STATUS_REG3_CMD                 0x11

#define PAGE_PROG_CMD                         0x02
#define QUAD_INPUT_PAGE_PROG_CMD              0x32

#define SECTOR_ERASE_CMD                      0x20
#define BLOCK_32KB_ERASE_CMD                  0x52
#define BLOCK_64KB_ERASE_CMD                  0xD8
#define CHIP_ERASE_CMD                        0xC7

void W25QXX_Init(void);
void W25QXX_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size);
void W25QXX_Write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size);
void W25QXX_EraseSector(uint32_t SectorAddress);
void W25QXX_EraseFullChip(void);
uint32_t W25QXX_ReadId(void);

#endif
