#include "qspi.h"

//�ȴ�״̬��־
//flag:��Ҫ�ȴ��ı�־λ
//sta:��Ҫ�ȴ���״̬
//wtime:�ȴ�ʱ��
//����ֵ:0,�ȴ��ɹ�.
//       1,�ȴ�ʧ��.
u8 QSPI_Wait_Flag(u32 flag, u8 sta, u32 wtime)
{
    u8 flagsta = 0;
    while(wtime)
    {
        flagsta = (OCTOSPI1->SR & flag) ? 1 : 0;
        if(flagsta == sta)break;
        wtime--;
    }
    if(wtime)return 0;
    else return 1;
}

//��ʼ��QSPI�ӿ�
//����ֵ:0,�ɹ�;
//       1,ʧ��;

/**OCTOSPI1 GPIO Configuration    
    PE2     ------> OCTOSPI1_BK1_IO2
    PD13    ------> OCTOSPI1_BK1_IO3
    PB2     ------> OCTOSPI1_CLK
    PB6     ------> OCTOSPI1_BK1_NCS
    PD12    ------> OCTOSPI1_BK1_IO1
    PD11    ------> OCTOSPI1_BK1_IO0 
    */

u8 QSPI_Init(void)
{
    u32 tempreg = 0;
    RCC->AHB4ENR |= 1 << 0;     //ʹ��PORTAʱ��
    RCC->AHB4ENR |= 1 << 1;     //ʹ��PORTBʱ��
    RCC->AHB4ENR |= 1 << 2;     //ʹ��PORTCʱ��
    RCC->AHB4ENR |= 1 << 3;     //ʹ��PORTDʱ��
    RCC->AHB4ENR |= 1 << 4;     //ʹ��PORTEʱ��
    
    RCC->AHB3ENR |= 1 << 14;    //QSPIʱ��ʹ��
    
    GPIO_Set(GPIOE, 1 << 2,  GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE); //PE2 ���ù������
    GPIO_Set(GPIOD, 1 << 13, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE); //PD13���ù������
    GPIO_Set(GPIOD, 1 << 12, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE); //PD12���ù������
    GPIO_Set(GPIOD, 1 << 11, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE); //PD11���ù������
    GPIO_Set(GPIOB, 1 << 2,  GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE); //PB2 ���ù������
    GPIO_Set(GPIOB, 1 << 6,  GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PUPD_PU); //PB6 ���ù������
    
    GPIO_AF_Set(GPIOE, 2, 9);   //PE2,AF9
    GPIO_AF_Set(GPIOB, 2, 9);   //PB2,AF9
    GPIO_AF_Set(GPIOB, 6, 10);  //PB6,AF10
    GPIO_AF_Set(GPIOD, 11, 9);  //PD11,AF9
    GPIO_AF_Set(GPIOD, 12, 9);  //PD12,AF9
    GPIO_AF_Set(GPIOD, 13, 9);  //PD13,AF9

    RCC->AHB3RSTR |= 1 << 14;   //��λQSPI
    RCC->AHB3RSTR &= ~(1 << 14); //ֹͣ��λQSPI
    if(QSPI_Wait_Flag(1 << 5, 0, 0XFFFF) == 0) //�ȴ�BUSY����
    {
        //QSPIʱ��Ĭ������rcc_hclk3(��RCC_D1CCIPR��QSPISEL[1:0]ѡ��)
        tempreg = (8 - 1) << 8; //����FIFO��ֵΪ8���ֽ�(���Ϊ31,��ʾ32���ֽ�)
        tempreg |= 0 << 7;      //ѡ��FLASH1
        tempreg |= 0 << 6;      //��ֹ˫����ģʽ
        OCTOSPI1->CR = tempreg;  //����CR�Ĵ���
				
        tempreg = (23 - 1) << 16; //����FLASH��СΪ2^23=8MB
        tempreg |= (5 - 1) << 8; //Ƭѡ�ߵ�ƽʱ��Ϊ5��ʱ��(10*5=50ns),���ֲ������tSHSL����
        tempreg |= 1 << 0;      //Mode3,����ʱCLKΪ�ߵ�ƽ
        OCTOSPI1->DCR1 = tempreg; //����DCR�Ĵ���
		
        tempreg = (2 - 1) << 0; //ʱ��2��Ƶ
        OCTOSPI1->DCR2 = tempreg; //����DCR�Ĵ���
		
        OCTOSPI1->DCR3 = 0; //����DCR�Ĵ���
		
        OCTOSPI1->DCR4 = 0; //����DCR�Ĵ���
		
        OCTOSPI1->CR |= 1 << 0;  //ʹ��QSPI
    }
    else return 1;
    return 0;
}

//QSPI��������
//cmd:Ҫ���͵�ָ��
//addr:���͵���Ŀ�ĵ�ַ
//mode:ģʽ,��ϸλ��������:
//  mode[1:0]:ָ��ģʽ;00,��ָ��;01,���ߴ���ָ��;10,˫�ߴ���ָ��;11,���ߴ���ָ��.
//  mode[3:2]:��ַģʽ;00,�޵�ַ;01,���ߴ����ַ;10,˫�ߴ����ַ;11,���ߴ����ַ.
//  mode[5:4]:��ַ����;00,8λ��ַ;01,16λ��ַ;10,24λ��ַ;11,32λ��ַ.
//  mode[7:6]:����ģʽ;00,������;01,���ߴ�������;10,˫�ߴ�������;11,���ߴ�������.
//dmcycle:��ָ��������
void QSPI_Send_CMD(u8 cmd, u32 addr, u8 mode, u8 dmcycle)
{
    u32 tempreg = 0;
    u8 status;
    if(QSPI_Wait_Flag(1 << 5, 0, 0XFFFF) == 0) //�ȴ�BUSY����
    {
		tempreg =OCTOSPI1->CR;
		tempreg &= ~(3 << 28);                		//���FMODEԭ��������
    	OCTOSPI1->CR = tempreg;                 	//��дCR�Ĵ���
		
        tempreg = 0 << 31;                 			//ÿ�ζ�����ָ��
        tempreg |= 0 << 29;                 		//��ֹDQSģʽ
        tempreg |= 0 << 27;                 		//��ֹDDRģʽ
        tempreg |= ((u32)mode >> 6) << 24;  		//��������ģʽ
        tempreg |= 0 << 16;                  		//�޽����ֽ�
        tempreg |= ((u32)(mode >> 4) & 0X03) << 12; //���õ�ַ����
        tempreg |= 0 << 11;                  		//��ֹ��ַDTR
        tempreg |= ((u32)(mode >> 2) & 0X03) << 8; 	//���õ�ַģʽ
        tempreg |= 0 << 4; 							//8λָ��
        tempreg |= 0 << 3;                  		//��ָֹ��DTR
        tempreg |= ((u32)(mode >> 0) & 0X03) << 0; 	//����ָ��ģʽ
		
        OCTOSPI1->CCR = tempreg;             		//����CCR�Ĵ���
		
        OCTOSPI1->TCR = dmcycle;      				//���ÿ�ָ��������
		
        OCTOSPI1->IR = cmd;                     	//����ָ��

		//OCTOSPI1->DR=0;
		if(mode & 0X0C)                     		//��ָ�� �е�ַҪ����
        {
           	OCTOSPI1->AR = addr;             		//���õ�ַ�Ĵ���
        }
		
        if((mode & 0XC0)==0)	              		//�����ݴ���,�ȴ�ָ������
        {
        	status = QSPI_Wait_Flag(1 << 1, 1, 0XFFFF); //�ȴ�TCF,���������
        	if(status == 0)
        	{
            	 OCTOSPI1->FCR |= 1 << 1;     //���TCF��־λ
        	}
		}
    }
}

//QSPI����ָ�����ȵ�����
//buf:�������ݻ������׵�ַ
//datalen:Ҫ��������ݳ���
//����ֵ:0,����
//    ����,�������
u8 QSPI_Receive(u8 *buf, u32 datalen)
{
    u32 tempreg;
    u8 status;
    vu32 *data_reg = &OCTOSPI1->DR;

	if(QSPI_Wait_Flag(1 << 5, 0, 0XFFFF) == 0) //�ȴ�BUSY����
	{
		
		OCTOSPI1->DLR = datalen - 1;             //�������ݴ��䳤��
		
		tempreg = OCTOSPI1->CR;
		tempreg &= ~(3 << 28);                	//���FMODEԭ��������
		tempreg |= 1 << 28;                   	//����FMODEΪ��Ӷ�ȡģʽ
		OCTOSPI1->CR = tempreg;                 //��дCR�Ĵ���

		OCTOSPI1->FCR |= 1 << 1;                //���TCF��־λ
		tempreg= OCTOSPI1->IR;
		OCTOSPI1->IR = tempreg;                  	//��дAR�Ĵ���,��������
		tempreg= OCTOSPI1->AR;
		OCTOSPI1->AR = tempreg;                  	//��дAR�Ĵ���,��������
		while(datalen)
		{
			status = QSPI_Wait_Flag(3 << 1, 1, 0XFFFF); //�ȵ�FTF&TCF,�����յ�������
			if(status == 0)                     //�ȴ��ɹ�
			{
				*buf++ = *(vu8 *)data_reg;
				datalen--;
			}
			else break;
		}
		if(status == 0)
		{
			OCTOSPI1->CR |= 1 << 1;                      //��ֹ����
			status = QSPI_Wait_Flag(1 << 1, 1, 0XFFFF); //�ȴ�TCF,�����ݴ������
			if(status == 0)
			{
				OCTOSPI1->FCR |= 1 << 1;                 //���TCF��־λ
				status = QSPI_Wait_Flag(1 << 5, 0, 0XFFFF); //�ȴ�BUSYλ����
			}
		}
	}
    return status;
}

//QSPI����ָ�����ȵ�����
//buf:�������ݻ������׵�ַ
//datalen:Ҫ��������ݳ���
//����ֵ:0,����
//    ����,�������
u8 QSPI_Transmit(u8 *buf, u32 datalen)
{
    u32 tempreg;
    u8 status;
    vu32 *data_reg = &OCTOSPI1->DR;
    
	OCTOSPI1->DLR = datalen - 1;             //�������ݴ��䳤��
	
	tempreg = OCTOSPI1->CR;
	tempreg &= ~(3 << 28);                	//���FMODEԭ��������
    tempreg |= 0 << 28;                   	//����FMODEΪ���дģʽ
	
    OCTOSPI1->CR = tempreg;                 //��дCR�Ĵ���
	
    OCTOSPI1->FCR |= 1 << 1;                 //���TCF��־λ
	tempreg= OCTOSPI1->IR;
	OCTOSPI1->IR = tempreg;                  	//��дAR�Ĵ���,��������
	
    while(datalen)
    {
        status = QSPI_Wait_Flag(1 << 2, 1, 0XFFFF); //�ȵ�FTF
        if(status != 0)                     //�ȴ��ɹ�
        {
            break;
        }
        *(vu8 *)data_reg = *buf++;
        datalen--;
    }
    if(status == 0)
    {
        OCTOSPI1->CR |= 1 << 1;                      //��ֹ����
        status = QSPI_Wait_Flag(1 << 1, 1, 0XFFFF); //�ȴ�TCF,�����ݴ������
        if(status == 0)
        {
            OCTOSPI1->FCR |= 1 << 1;                 //���TCF��־λ
            status = QSPI_Wait_Flag(1 << 5, 0, 0XFFFF); //�ȴ�BUSYλ����
        }
    }
    return status;
}

void QSPI_MMAP(void)
{
    u32 tempreg;

    tempreg = OCTOSPI1->CR;
    tempreg &= ~(3 << 28); 
    tempreg |= (3 << 28);
    OCTOSPI1->CR = tempreg; 
    tempreg = (3 << 24);
    tempreg |= (2 << 12);
    tempreg |= (1 << 8);
    tempreg |= (1 << 0);
    OCTOSPI1->CCR = tempreg;
    OCTOSPI1->IR = 0x6b;
    OCTOSPI1->TCR = 8;
}

void QSPI_Abort(void)
{
    OCTOSPI1->CR |= (1 << 1);
    while((OCTOSPI1->SR & (1 << 5)));
}