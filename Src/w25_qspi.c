/*
 * w25_qspi.c
 *
 *  Created on: Apr 13, 2022
 *      Author: zjm09
 */
#include "stm32h750xx.h"
#include "sys.h"

/* QUADSPI_DCR */
#define QSPI_DCR_CSHT(x)			((x) << 8)
#define QSPI_DCR_FSIZE(x)		((x) << 16)

#define QUADSPI_DCR_CSHT_MASK		QSPI_DCR_CSHT(0x7)
#define QUADSPI_DCR_FSIZE_MASK		QSPI_DCR_FSIZE(0x1f)
#define QUADSPI_DCR_FSIZE_8MB		QSPI_DCR_FSIZE(22)
#define QUADSPI_DCR_FSIZE_16MB		QSPI_DCR_FSIZE(23)
#define QUADSPI_DCR_FSIZE_32MB		QSPI_DCR_FSIZE(24)
#define QUADSPI_DCR_FSIZE_64MB		QSPI_DCR_FSIZE(25)
#define QUADSPI_DCR_FSIZE_128MB		QSPI_DCR_FSIZE(26)

#define QSPI_CCR_IDMODE(x)		((x) << 8)
#define QSPI_CCR_ADMODE(x)		((x) << 10)
#define QSPI_CCR_ADSIZE(x)		((x) << 12)
#define QSPI_CCR_DCYC(x)			((x) << 18)
#define QSPI_CCR_DMODE(x)		((x) << 24)
#define QSPI_CCR_FMODE(x)		((x) << 26)

#define QUADSPI_CCR_IDMOD_1_LINE	QSPI_CCR_IDMODE(1)
#define QUADSPI_CCR_ADMOD_1_LINE	QSPI_CCR_ADMODE(1)
#define QUADSPI_CCR_ADSIZE_24BITS	QSPI_CCR_ADSIZE(2)
#define QUADSPI_CCR_ADSIZE_32BITS	QSPI_CCR_ADSIZE(3)
#define QUADSPI_CCR_DMODE_1_LINE	QSPI_CCR_DMODE(1)
#define QUADSPI_CCR_DMODE_4_LINES	QSPI_CCR_DMODE(3)
#define QUADSPI_CCR_FMODE_IND_WR	QSPI_CCR_FMODE(0)
#define QUADSPI_CCR_FMODE_IND_RD	QSPI_CCR_FMODE(1)
#define QUADSPI_CCR_FMODE_AUTO_POLL	QSPI_CCR_FMODE(2)
#define QUADSPI_CCR_FMODE_MEMMAP	QSPI_CCR_FMODE(3)

/*  QSPI Comands */
#define READ_STATUS_REG_CMD			0x05
#define WRITE_ENABLE_CMD			0x06
#define RESET_ENABLE_CMD			0x66
#define QUAD_OUTPUT_FAST_READ_CMD	0x6b
#define WRITE_VOL_CFG_REG_CMD		0x81
#define READ_VOL_CFG_REG_CMD		0x85
#define RESET_MEMORY_CMD			0x99
#define ENTER_4_BYTE_ADDR_MODE_CMD	0xb7

/* W25Q512A_SR */
#define W25Q512A_SR_WIP				(1 << 0)
#define W25Q512A_SR_WREN			(1 << 1)

void qspi_busy_wait(void)
{
	while((QUADSPI->SR & QUADSPI_SR_BUSY));
}

void qspi_wait_flag(uint32_t flag)
{
	while (!(QUADSPI->SR & flag));
	QUADSPI->FCR = flag;
}

void qspi_write_enable(void)
{
	qspi_busy_wait();

	QUADSPI->CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_IDMOD_1_LINE | WRITE_ENABLE_CMD;

	qspi_wait_flag(QUADSPI_SR_TCF);

	qspi_busy_wait();

	QUADSPI->PSMAR = W25Q512A_SR_WREN;
	QUADSPI->PSMKR = W25Q512A_SR_WREN;
	QUADSPI->PIR = 16;

	QUADSPI->CR |= QUADSPI_CR_APMS;
	QUADSPI->DLR = 0;

	QUADSPI->CCR = QUADSPI_CCR_FMODE_AUTO_POLL | QUADSPI_CCR_DMODE_1_LINE |
			QUADSPI_CCR_IDMOD_1_LINE | READ_STATUS_REG_CMD;

	qspi_wait_flag(QUADSPI_SR_SMF);
}

void qspi_memory_ready(void)
{
	qspi_busy_wait();

	QUADSPI->PSMAR = 0;
	QUADSPI->PSMKR = W25Q512A_SR_WIP;
	QUADSPI->PIR = 16;

	QUADSPI->CR |= QUADSPI_CR_APMS;
	QUADSPI->DLR = 0;
	QUADSPI->CCR = QUADSPI_CCR_FMODE_AUTO_POLL | QUADSPI_CCR_DMODE_1_LINE |
		QUADSPI_CCR_IDMOD_1_LINE | READ_STATUS_REG_CMD;

	qspi_wait_flag(QUADSPI_SR_SMF);
}

void qspi_erase_sector(uint32_t sector)
{
	QUADSPI->CR |= QUADSPI_CR_ABORT;

	qspi_busy_wait();

	qspi_write_enable();

	qspi_busy_wait();

	QUADSPI->CCR = QUADSPI_CCR_FMODE_IND_WR |
                QUADSPI_CCR_ADSIZE_24BITS |
				QUADSPI_CCR_ADMOD_1_LINE |
                QUADSPI_CCR_IDMOD_1_LINE | 0x20;

	QUADSPI->AR = sector;

	qspi_wait_flag(QUADSPI_SR_TCF);

	qspi_memory_ready();
}

void qspi_write(uint32_t address,uint8_t *data,int len)
{
  int txCount;
  volatile uint32_t *data_reg = &QUADSPI->DR;

  QUADSPI->CR |= QUADSPI_CR_ABORT;
  qspi_busy_wait();

  while(len > 0){
    qspi_write_enable();

    qspi_busy_wait();

    QUADSPI->DLR = 255;
    QUADSPI->CCR = QUADSPI_CCR_FMODE_IND_WR |
		QSPI_CCR_DCYC(0) | QUADSPI_CCR_ADSIZE_24BITS | QUADSPI_CCR_DMODE_4_LINES |
		QUADSPI_CCR_ADMOD_1_LINE | QUADSPI_CCR_IDMOD_1_LINE | 0x32;
    QUADSPI->AR = address;

    txCount = 256;
    while(txCount-- > 0){
      qspi_wait_flag(QUADSPI_SR_FTF);
       *(volatile uint8_t *)data_reg = *data ++;
    }

    qspi_wait_flag(QUADSPI_SR_TCF);

    len -= 256;
    address += 256;

    qspi_memory_ready();
  }
}

void qspi_reset_memory(void)
{
	/* Reset memory */
	qspi_busy_wait();

	QUADSPI->CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_IDMOD_1_LINE |
		RESET_ENABLE_CMD;

	qspi_wait_flag(QUADSPI_SR_TCF);

	QUADSPI->CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_IDMOD_1_LINE |
		RESET_MEMORY_CMD;

	qspi_wait_flag(QUADSPI_SR_TCF);
}

void qspi_mmap(void)
{
	QUADSPI->CCR = QUADSPI_CCR_FMODE_MEMMAP | QUADSPI_CCR_DMODE_4_LINES |
		QSPI_CCR_DCYC(8) | QUADSPI_CCR_ADSIZE_24BITS |
		QUADSPI_CCR_ADMOD_1_LINE | QUADSPI_CCR_IDMOD_1_LINE |
		QUAD_OUTPUT_FAST_READ_CMD;

	qspi_busy_wait();
}

void qspi_init(void)
{
	/* QUADSPI clock enable */
	RCC->AHB4ENR|=1<<1; //使能GPIOB时钟
	RCC->AHB4ENR|=1<<3; //使能GPIOD时钟
	RCC->AHB4ENR|=1<<4;	//使能GPIOE时钟
	RCC->AHB3ENR|=1<<14; //QSPI时钟使能

	RCC->D1CCIPR = (0<<4);

	GPIO_Set(GPIOB, PIN6, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_FAST, GPIO_PUPD_NONE);
	GPIO_Set(GPIOB, PIN2, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_FAST, GPIO_PUPD_NONE);
	GPIO_Set(GPIOE, PIN2, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_FAST, GPIO_PUPD_NONE);
	GPIO_Set(GPIOD, PIN11, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_FAST, GPIO_PUPD_NONE);
	GPIO_Set(GPIOD, PIN12, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_FAST, GPIO_PUPD_NONE);
	GPIO_Set(GPIOD, PIN13, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_FAST, GPIO_PUPD_NONE);

	GPIO_AF_Set(GPIOB, 6, 10);
	GPIO_AF_Set(GPIOB, 2, 9);
	GPIO_AF_Set(GPIOE, 2, 9);
	GPIO_AF_Set(GPIOD, 11, 9);
	GPIO_AF_Set(GPIOD, 12, 9);
	GPIO_AF_Set(GPIOD, 13, 9);


    RCC->AHB3RSTR |= 1 << 14;   //复位QSPI
    RCC->AHB3RSTR &= ~(1 << 14); //停止复位QSPI

	qspi_busy_wait();

	QUADSPI->CR = QUADSPI_CR_FTHRES_0;
    QUADSPI->CR |= QUADSPI_CR_PRESCALER_2 | QUADSPI_CR_SSHIFT;
    QUADSPI->DCR = QUADSPI_DCR_FSIZE_8MB | QUADSPI_DCR_CSHT_2;

    QUADSPI->CR |= QUADSPI_CR_EN;

    qspi_reset_memory();

    qspi_memory_ready();
}
