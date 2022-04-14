/*
 * sys.c
 *
 *  Created on: Apr 14, 2022
 *      Author: zjm09
 */
#include "stm32h750xx.h"

uint8_t Sys_Clock_Set(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    uint16_t retry = 0;
    uint8_t status = 0;

    PWR->CR3 &= ~(1 << 2);          //SCUEN=0,����LDOEN��BYPASSλ������
    PWR->D3CR |= 3 << 14;           //VOS=3,Scale1,1.15~1.26V�ں˵�ѹ,FLASH���ʿ��Եõ��������
    while((PWR->D3CR & (1 << 13)) == 0); //�ȴ���ѹ�ȶ�

    RCC->CR |= 1 << 0;          //����HSI
    RCC->ICSCR &= ~(32 << 12);  //HSIУ׼ֵ����
    RCC->ICSCR |= 16 << 12; //HSIУ׼ֵΪ16
    while((RCC->CR & (1 << 2)) == 0); //�ȴ�HSI�����ȶ�

    RCC->CR |= 3 << 3;          //HSIDIV[1:0]=3,HSI���8MHz
    RCC->PLLCKSELR |= 0 << 0;   //PLLSRC[1:0]=0,ѡ��HSI��ΪPLL������ʱ��Դ
    RCC->PLLCKSELR |= pllm << 4; //DIVM1[5:0]=pllm,����PLL1��Ԥ��Ƶϵ��
    RCC->PLL1DIVR &= ~((129 - 1) << 0); //��������plln
    RCC->PLL1DIVR |= (plln - 1) << 0; //DIVN1[8:0]=plln-1,����PLL1�ı�Ƶϵ��,����ֵ���1
    RCC->PLL1DIVR |= (pllp - 1) << 9; //DIVP1[6:0]=pllp-1,����PLL1��p��Ƶϵ��,����ֵ���1
    RCC->PLL1DIVR |= (pllq - 1) << 16; //DIVQ1[6:0]=pllq-1,����PLL1��q��Ƶϵ��,����ֵ���1
    RCC->PLL1DIVR |= 1 << 24;   //DIVR1[6:0]=pllr-1,����PLL1��r��Ƶϵ��,����ֵ���1,r��Ƶ������ʱ��û�õ�
    RCC->PLLCFGR |= 2 << 2;     //PLL1RGE[1:0]=2,PLL1����ʱ��Ƶ����4~8Mhz֮��(25/5=5Mhz),���޸�pllm,��ȷ�ϴ˲���
    RCC->PLLCFGR |= 0 << 1;     //PLL1VCOSEL=0,PLL1���VCO��Χ,192~836Mhz
    RCC->PLLCFGR |= 3 << 16;    //DIVP1EN=1,DIVQ1EN=1,ʹ��pll1_p_ck��pll1_q_ck
    RCC->CR |= 1 << 24;         //PLL1ON=1,ʹ��PLL1
    while((RCC->CR & (1 << 25)) == 0); //PLL1RDY=1?,�ȴ�PLL1׼����

    RCC->D1CFGR |= 8 << 0;      //HREF[3:0]=8,rcc_hclk1/2/3/4=sys_d1cpre_ck/2=400/2=200Mhz,��AHB1/2/3/4=200Mhz
    RCC->D1CFGR |= 0 << 8;      //D1CPRE[2:0]=0,sys_d1cpre_ck=sys_clk/1=400/1=400Mhz,��CPUʱ��=400Mhz
    RCC->CFGR |= 3 << 0;        //SW[2:0]=3,ϵͳʱ��(sys_clk)ѡ������pll1_p_ck,��400Mhz
    while(1)
    {
        retry = (RCC->CFGR & (7 << 3)) >> 3; //��ȡSWS[2:0]��״̬,�ж��Ƿ��л��ɹ�
        if(retry == 3)break;    //�ɹ���ϵͳʱ��Դ�л�Ϊpll1_p_ck
    }

    FLASH->ACR |= 2 << 0;       //LATENCY[2:0]=2,2��CPU�ȴ�����(@VOS1 Level,maxclock=210Mhz)
    FLASH->ACR |= 2 << 4;       //WRHIGHFREQ[1:0]=2,flash����Ƶ��<285Mhz

    RCC->D1CFGR |= 4 << 4;      //D1PPRE[2:0]=4,rcc_pclk3=rcc_hclk3/2=100Mhz,��APB3=100Mhz
    RCC->D2CFGR |= 4 << 4;      //D2PPRE1[2:0]=4,rcc_pclk1=rcc_hclk1/2=100Mhz,��APB1=100Mhz
    RCC->D2CFGR |= 4 << 8;      //D2PPRE2[2:0]=4,rcc_pclk2=rcc_hclk1/2=100Mhz,��APB2=100Mhz
    RCC->D3CFGR |= 4 << 4;      //D3PPRE[2:0]=4,rcc_pclk4=rcc_hclk4/2=100Mhz,��APB4=100Mhz

    RCC->CR |= 1 << 7;          //CSION=1,ʹ��CSI,ΪIO������Ԫ�ṩʱ��
    RCC->APB4ENR |= 1 << 1;     //SYSCFGEN=1,ʹ��SYSCFGʱ��
    SYSCFG->CCCSR |= 1 << 0;    //EN=1,ʹ��IO������Ԫ

    return status;
}

void Stm32_Clock_Init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    RCC->CR = 0x00000001;           //����HSION,�����ڲ�����RC�񵴣�����λȫ����
    RCC->CFGR = 0x00000000;         //CFGR����
    RCC->D1CFGR = 0x00000000;       //D1CFGR����
    RCC->D2CFGR = 0x00000000;       //D2CFGR����
    RCC->D3CFGR = 0x00000000;       //D3CFGR����
    RCC->PLLCKSELR = 0x00000000;    //PLLCKSELR����
    RCC->PLLCFGR = 0x00000000;      //PLLCFGR����
    RCC->CIER = 0x00000000;         //CIER����,��ֹ����RCC����ж�
    //AXI_TARG7_FN_MOD�Ĵ���,����û����stm32h743xx.h���涨��,����,ֻ����ֱ��
    //������ַ�ķ�ʽ,���޸�,�üĴ�����<<STM32H7xx�ο��ֲ�>>��212ҳ,table 23
    *((volatile uint16_t *)0x51008108) = 0x00000001; //����AXI SRAM�ľ����ȡ����Ϊ1
//    Cache_Enable();                 //ʹ��L1 Cache
    Sys_Clock_Set(plln, pllm, pllp, pllq); //����ʱ��
    //����������
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
        pos = 1 << pinpos; //һ����λ���
        curpin = BITx & pos; //��������Ƿ�Ҫ����
        if(curpin == pos) //��Ҫ����
        {
            GPIOx->MODER &= ~(3 << (pinpos * 2)); //�����ԭ��������
            GPIOx->MODER |= MODE << (pinpos * 2); //�����µ�ģʽ
            if((MODE == 0X01) || (MODE == 0X02)) //��������ģʽ/���ù���ģʽ
            {
                GPIOx->OSPEEDR &= ~(3 << (pinpos * 2)); //���ԭ��������
                GPIOx->OSPEEDR |= (OSPEED << (pinpos * 2)); //�����µ��ٶ�ֵ
                GPIOx->OTYPER &= ~(1 << pinpos) ;   //���ԭ��������
                GPIOx->OTYPER |= OTYPE << pinpos;   //�����µ����ģʽ
            }
            GPIOx->PUPDR &= ~(3 << (pinpos * 2)); //�����ԭ��������
            GPIOx->PUPDR |= PUPD << (pinpos * 2); //�����µ�������
        }
    }
}

void SystemInit(void)
{

}

