/*
 * sys.c
 *
 *  Created on: Apr 14, 2022
 *      Author: zjm09
 */
#include "sys.h"

uint8_t Sys_Clock_Set(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    u16 retry = 0;
    u8 status = 0;

    PWR->CR3 &= ~(1 << 2);          //SCUEN=0,锁定LDOEN和BYPASS位的设置
    PWR->SRDCR |= 2 << 14;           //VOS=2,Scale1,1.15~1.26V内核电压,FLASH访问可以得到最高性能
    while((PWR->SRDCR & (1 << 13)) == 0); //等待电压稳定

    RCC->CR |= 1 << 0;          //开启HSI
    RCC->HSICFGR &= ~(128 << 24);  //HSI校准值清零
    RCC->HSICFGR |= 64 << 24; //HSI校准值为64
    while((RCC->CR & (1 << 2)) == 0); //等待HSI启动稳定

    RCC->CR |= 3 << 3;          //HSIDIV[1:0]=3,HSI输出8MHz
    RCC->PLLCKSELR |= 32 << 20; //DIVM3[5:0]=32,设置PLL3的预分频系数
    RCC->PLLCKSELR |= 32 << 12; //DIVM2[5:0]=32,设置PLL2的预分频系数
    RCC->PLLCKSELR |= pllm << 4; //DIVM1[5:0]=pllm,设置PLL1的预分频系数
    RCC->PLL1DIVR &= ~((512 - 1) << 0); //首先清零plln
    RCC->PLL1DIVR |= (plln - 1) << 0; //DIVN1[8:0]=plln-1,设置PLL1的倍频系数,设置值需减1
    RCC->PLL1DIVR |= (pllp - 1) << 9; //DIVP1[6:0]=pllp-1,设置PLL1的p分频系数,设置值需减1
    RCC->PLL1DIVR |= (pllq - 1) << 16; //DIVQ1[6:0]=pllq-1,设置PLL1的q分频系数,设置值需减1
    RCC->PLL1DIVR |= 1 << 24;   //DIVR1[6:0]=pllr-1,设置PLL1的r分频系数,设置值需减1,r分频出来的时钟没用到
    RCC->PLLCFGR |= 0 << 2;     //PLL1RGE[1:0]=2,PLL1输入时钟频率在4~8Mhz之间(64/32=2Mhz),如修改pllm,请确认此参数
    RCC->PLLCFGR |= 0 << 1;     //PLL1VCOSEL=0,PLL1宽的VCO范围,192~836Mhz
    RCC->PLLCFGR |= 0x01FF << 16; //DIVP1EN=1,DIVQ1EN=1,使能pll1_p_ck和pll1_q_ck

    RCC->CDCFGR1 |= 8 << 0;      //HREF[3:0]=8,rcc_hclk1/2/3/4=sys_d1cpre_ck/2=400/2=200Mhz,即AHB1/2/3/4=200Mhz
    RCC->CDCFGR1 |= 8 << 8;      //D1CPRE[2:0]=8,sys_d1cpre_ck=sys_clk/2=400/2=20Mhz,即CPU时钟=200Mhz
	
    RCC->CR |= 1 << 24;         //PLL1ON=1,使能PLL1
    while((RCC->CR & (1 << 25)) == 0); //PLL1RDY=1?,等待PLL1准备好

    RCC->CFGR |= 3 << 0;        //SW[2:0]=3,系统时钟(sys_clk)选择来自pll1_p_ck,即400Mhz
    while(1)
    {
        retry = (RCC->CFGR & (7 << 3)) >> 3; //获取SWS[2:0]的状态,判断是否切换成功
        if(retry == 3)break;    //成功将系统时钟源切换为pll1_p_ck
    }

    FLASH->ACR |= 2 << 0;       //LATENCY[2:0]=2,2个CPU等待周期(@VOS1 Level,maxclock=210Mhz)
    FLASH->ACR |= 2 << 4;       //WRHIGHFREQ[1:0]=2,flash访问频率<285Mhz

    RCC->CDCFGR1 |= 4 << 4;      //D1PPRE[2:0]=4,rcc_pclk3=rcc_hclk3/2=100Mhz,即APB3=100Mhz
    RCC->CDCFGR2 |= 4 << 4;      //D2PPRE1[2:0]=4,rcc_pclk1=rcc_hclk1/2=100Mhz,即APB1=100Mhz
    RCC->CDCFGR2 |= 4 << 8;      //D2PPRE2[2:0]=4,rcc_pclk2=rcc_hclk1/2=100Mhz,即APB2=100Mhz
    RCC->SRDCFGR |= 4 << 4;      //D3PPRE[2:0]=4,rcc_pclk4=rcc_hclk4/2=100Mhz,即APB4=100Mhz

    RCC->CR |= 1 << 7;          //CSION=1,使能CSI,为IO补偿单元提供时钟
    RCC->APB4ENR |= 1 << 1;     //SYSCFGEN=1,使能SYSCFG时钟
    SYSCFG->CCCSR |= 1 << 0;    //EN=1,使能IO补偿单元

    return status;
}

void Stm32_Clock_Init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    RCC->CR = 0x00000001;           //设置HSION,开启内部高速RC振荡，其他位全清零
    RCC->CFGR = 0x00000000;         //CFGR清零
    RCC->CDCFGR1 = 0x00000000;       //D1CFGR清零
    RCC->CDCFGR2 = 0x00000000;       //D2CFGR清零
    RCC->SRDCFGR = 0x00000000;       //D3CFGR清零
    RCC->PLLCKSELR = 0x00000000;    //PLLCKSELR清零
    RCC->PLLCFGR = 0x00000000;      //PLLCFGR清零
    RCC->CIER = 0x00000000;         //CIER清零,禁止所有RCC相关中断
    //AXI_TARG7_FN_MOD寄存器,由于没有在stm32h743xx.h里面定义,所以,只能用直接
    //操作地址的方式,来修改,该寄存器在<<STM32H7xx参考手册>>第212页,table 23
    *((vu32 *)0x51008008) = 0x00000001; //设置AXI SRAM的矩阵读取能力为1
    //Cache_Enable();                 //使能L1 Cache
    Sys_Clock_Set(plln, pllm, pllp, pllq); //设置时钟
    //配置向量表
//#ifdef  VECT_TAB_RAM
//    MY_NVIC_SetVectorTable(D1_AXISRAM_BASE, 0x0);
//#else
//    MY_NVIC_SetVectorTable(FLASH_BANK1_BASE, 0x0);
//#endif
}

void GPIO_AF_Set(GPIO_TypeDef *GPIOx, uint8_t BITx, uint8_t AFx)
{
    GPIOx->AFR[BITx >> 3] &= ~(0X0F << ((BITx & 0X07) * 4));
    GPIOx->AFR[BITx >> 3] |= (uint32_t)AFx << ((BITx & 0X07) * 4);
}

void GPIO_Set(GPIO_TypeDef *GPIOx, uint32_t BITx, uint32_t MODE, uint32_t OTYPE, uint32_t OSPEED, uint32_t PUPD)
{
    uint32_t pinpos = 0, pos = 0, curpin = 0;
    for(pinpos = 0; pinpos < 16; pinpos++)
    {
        pos = 1 << pinpos; //一个个位检查
        curpin = BITx & pos; //检查引脚是否要设置
        if(curpin == pos) //需要设置
        {
            GPIOx->MODER &= ~(3 << (pinpos * 2)); //先清除原来的设置
            GPIOx->MODER |= MODE << (pinpos * 2); //设置新的模式
            if((MODE == 0X01) || (MODE == 0X02)) //如果是输出模式/复用功能模式
            {
                GPIOx->OSPEEDR &= ~(3 << (pinpos * 2)); //清除原来的设置
                GPIOx->OSPEEDR |= (OSPEED << (pinpos * 2)); //设置新的速度值
                GPIOx->OTYPER &= ~(1 << pinpos) ;   //清除原来的设置
                GPIOx->OTYPER |= OTYPE << pinpos;   //设置新的输出模式
            }
            GPIOx->PUPDR &= ~(3 << (pinpos * 2)); //先清除原来的设置
            GPIOx->PUPDR |= PUPD << (pinpos * 2); //设置新的上下拉
        }
    }
}

void SystemInit(void)
{

}

