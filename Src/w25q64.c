#include "w25q64.h"

uint8_t W25QXX_ReadREG1(void)
{
    u8 byte = 0;
    QSPI_Send_CMD(READ_STATUS_REG1_CMD, 0, (1 << 6) | (0 << 4) | (0 << 2) | (1 << 0), 0);
    QSPI_Receive(&byte, 1);
    return byte;
}

void W25QXX_WaitBusy(void)
{
    while((W25QXX_ReadREG1() & 0x01) == 0x01);
}

void W25QXX_ResetMemory(void)
{
    QSPI_Send_CMD(RESET_ENABLE_CMD, 0, (0 << 6) | (0 << 4) | (0 << 2) | (1 << 0), 0);
    QSPI_Send_CMD(RESET_MEMORY_CMD, 0, (0 << 6) | (0 << 4) | (0 << 2) | (1 << 0), 0);
    W25QXX_WaitBusy();
}

void W25QXX_WriteEnable(void)
{
    QSPI_Send_CMD(WRITE_ENABLE_CMD, 0, (0 << 6) | (0 << 4) | (0 << 2) | (1 << 0), 0);
}

void W25QXX_Init(void)
{
    uint8_t value = 0x02;
    W25QXX_ResetMemory();
    W25QXX_WriteEnable();
    QSPI_Send_CMD(WRITE_STATUS_REG2_CMD, 0, (1 << 6) | (0 << 4) | (0 << 2) | (1 << 0), 0);
    QSPI_Transmit(&value, 1);
    W25QXX_WaitBusy();
}

void W25QXX_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
    QSPI_Send_CMD(QUAD_INOUT_FAST_READ_CMD, ReadAddr, (3 << 6) | (2 << 4) | (3 << 2) | (1 << 0), 6);
    QSPI_Receive(pData, Size);
}

void W25QXX_Write_Page(u8 *pBuffer, u32 WriteAddr, u32 NumByteToWrite)
{
    W25QXX_WriteEnable();
    QSPI_Send_CMD(QUAD_INPUT_PAGE_PROG_CMD, WriteAddr, (3 << 6) | (2 << 4) | (1 << 2) | (1 << 0), 0);
    QSPI_Transmit(pBuffer, NumByteToWrite);
    W25QXX_WaitBusy();
}

void W25QXX_Write(uint8_t *pData, uint32_t WriteAddr, uint32_t NumByteToWrite)
{
    u16 pageremain;
    pageremain = 256 - WriteAddr % 256; //单页剩余的字节数
    if(NumByteToWrite <= pageremain)pageremain = NumByteToWrite; //不大于256个字节
    QSPI_Abort();
    while(1)
    {
        W25QXX_Write_Page(pData, WriteAddr, pageremain);
        if(NumByteToWrite == pageremain)break; //写入结束了
        else //NumByteToWrite>pageremain
        {
            pData += pageremain;
            WriteAddr += pageremain;

            NumByteToWrite -= pageremain;         //减去已经写入了的字节数
            if(NumByteToWrite > 256)pageremain = 256; //一次可以写入256个字节
            else pageremain = NumByteToWrite;     //不够256个字节了
        }
    }
}

void W25QXX_EraseSector(uint32_t SectorAddress)
{
    QSPI_Abort();
    /* 写使能 */
    W25QXX_WriteEnable();
    /* 发送命令 */
    QSPI_Send_CMD(SECTOR_ERASE_CMD, SectorAddress, (0 << 6) | (2 << 4) | (1 << 2) | (1 << 0), 0);

    /* 自动轮询模式等待动作完成 */
    W25QXX_WaitBusy();
}

void W25QXX_EraseFullChip(void)
{
    QSPI_Abort();
    /* 写使能 */
    W25QXX_WriteEnable();

    /* 发送指令 */
    QSPI_Send_CMD(CHIP_ERASE_CMD, 0, (0 << 6) | (0 << 4) | (0 << 2) | (1 << 0), 0);

    /* 自动轮询模式等待动作完成 */
    W25QXX_WaitBusy();
}

uint32_t W25QXX_ReadId(void)
{
    uint8_t pData[3];
    uint32_t device_id;

    QSPI_Send_CMD(READ_JEDEC_ID_CMD, 0, (1 << 6) | (0 << 4) | (0 << 2) | (1 << 0), 0);
    QSPI_Receive(pData, 3);

    device_id = pData[2] | (pData[1] << 8) | (pData[0] << 16);
    return device_id;
}
