#include "qspi.h"

//等待状态标志
//flag:需要等待的标志位
//sta:需要等待的状态
//wtime:等待时间
//返回值:0,等待成功.
//       1,等待失败.
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

//初始化QSPI接口
//返回值:0,成功;
//       1,失败;

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
    RCC->AHB4ENR |= 1 << 0;     //使能PORTA时钟
    RCC->AHB4ENR |= 1 << 1;     //使能PORTB时钟
    RCC->AHB4ENR |= 1 << 2;     //使能PORTC时钟
    RCC->AHB4ENR |= 1 << 3;     //使能PORTD时钟
    RCC->AHB4ENR |= 1 << 4;     //使能PORTE时钟
    
    RCC->AHB3ENR |= 1 << 14;    //QSPI时钟使能
    
    GPIO_Set(GPIOE, 1 << 2,  GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE); //PE2 复用功能输出
    GPIO_Set(GPIOD, 1 << 13, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE); //PD13复用功能输出
    GPIO_Set(GPIOD, 1 << 12, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE); //PD12复用功能输出
    GPIO_Set(GPIOD, 1 << 11, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE); //PD11复用功能输出
    GPIO_Set(GPIOB, 1 << 2,  GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE); //PB2 复用功能输出
    GPIO_Set(GPIOB, 1 << 6,  GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PUPD_PU); //PB6 复用功能输出
    
    GPIO_AF_Set(GPIOE, 2, 9);   //PE2,AF9
    GPIO_AF_Set(GPIOB, 2, 9);   //PB2,AF9
    GPIO_AF_Set(GPIOB, 6, 10);  //PB6,AF10
    GPIO_AF_Set(GPIOD, 11, 9);  //PD11,AF9
    GPIO_AF_Set(GPIOD, 12, 9);  //PD12,AF9
    GPIO_AF_Set(GPIOD, 13, 9);  //PD13,AF9

    RCC->AHB3RSTR |= 1 << 14;   //复位QSPI
    RCC->AHB3RSTR &= ~(1 << 14); //停止复位QSPI
    if(QSPI_Wait_Flag(1 << 5, 0, 0XFFFF) == 0) //等待BUSY空闲
    {
        //QSPI时钟默认来自rcc_hclk3(由RCC_D1CCIPR的QSPISEL[1:0]选择)
        tempreg = (8 - 1) << 8; //设置FIFO阈值为8个字节(最大为31,表示32个字节)
        tempreg |= 0 << 7;      //选择FLASH1
        tempreg |= 0 << 6;      //禁止双闪存模式
        OCTOSPI1->CR = tempreg;  //设置CR寄存器
				
        tempreg = (23 - 1) << 16; //设置FLASH大小为2^23=8MB
        tempreg |= (5 - 1) << 8; //片选高电平时间为5个时钟(10*5=50ns),即手册里面的tSHSL参数
        tempreg |= 1 << 0;      //Mode3,空闲时CLK为高电平
        OCTOSPI1->DCR1 = tempreg; //设置DCR寄存器
		
        tempreg = (2 - 1) << 0; //时钟2分频
        OCTOSPI1->DCR2 = tempreg; //设置DCR寄存器
		
        OCTOSPI1->DCR3 = 0; //设置DCR寄存器
		
        OCTOSPI1->DCR4 = 0; //设置DCR寄存器
		
        OCTOSPI1->CR |= 1 << 0;  //使能QSPI
    }
    else return 1;
    return 0;
}

//QSPI发送命令
//cmd:要发送的指令
//addr:发送到的目的地址
//mode:模式,详细位定义如下:
//  mode[1:0]:指令模式;00,无指令;01,单线传输指令;10,双线传输指令;11,四线传输指令.
//  mode[3:2]:地址模式;00,无地址;01,单线传输地址;10,双线传输地址;11,四线传输地址.
//  mode[5:4]:地址长度;00,8位地址;01,16位地址;10,24位地址;11,32位地址.
//  mode[7:6]:数据模式;00,无数据;01,单线传输数据;10,双线传输数据;11,四线传输数据.
//dmcycle:空指令周期数
void QSPI_Send_CMD(u8 cmd, u32 addr, u8 mode, u8 dmcycle)
{
    u32 tempreg = 0;
    u8 status;
    if(QSPI_Wait_Flag(1 << 5, 0, 0XFFFF) == 0) //等待BUSY空闲
    {
		tempreg =OCTOSPI1->CR;
		tempreg &= ~(3 << 28);                		//清除FMODE原来的设置
    	OCTOSPI1->CR = tempreg;                 	//回写CR寄存器
		
        tempreg = 0 << 31;                 			//每次都发送指令
        tempreg |= 0 << 29;                 		//禁止DQS模式
        tempreg |= 0 << 27;                 		//禁止DDR模式
        tempreg |= ((u32)mode >> 6) << 24;  		//设置数据模式
        tempreg |= 0 << 16;                  		//无交替字节
        tempreg |= ((u32)(mode >> 4) & 0X03) << 12; //设置地址长度
        tempreg |= 0 << 11;                  		//禁止地址DTR
        tempreg |= ((u32)(mode >> 2) & 0X03) << 8; 	//设置地址模式
        tempreg |= 0 << 4; 							//8位指令
        tempreg |= 0 << 3;                  		//禁止指令DTR
        tempreg |= ((u32)(mode >> 0) & 0X03) << 0; 	//设置指令模式
		
        OCTOSPI1->CCR = tempreg;             		//设置CCR寄存器
		
        OCTOSPI1->TCR = dmcycle;      				//设置空指令周期数
		
        OCTOSPI1->IR = cmd;                     	//设置指令

		//OCTOSPI1->DR=0;
		if(mode & 0X0C)                     		//有指令 有地址要发送
        {
           	OCTOSPI1->AR = addr;             		//设置地址寄存器
        }
		
        if((mode & 0XC0)==0)	              		//无数据传输,等待指令发送完成
        {
        	status = QSPI_Wait_Flag(1 << 1, 1, 0XFFFF); //等待TCF,即传输完成
        	if(status == 0)
        	{
            	 OCTOSPI1->FCR |= 1 << 1;     //清除TCF标志位
        	}
		}
    }
}

//QSPI接收指定长度的数据
//buf:接收数据缓冲区首地址
//datalen:要传输的数据长度
//返回值:0,正常
//    其他,错误代码
u8 QSPI_Receive(u8 *buf, u32 datalen)
{
    u32 tempreg;
    u8 status;
    vu32 *data_reg = &OCTOSPI1->DR;

	if(QSPI_Wait_Flag(1 << 5, 0, 0XFFFF) == 0) //等待BUSY空闲
	{
		
		OCTOSPI1->DLR = datalen - 1;             //设置数据传输长度
		
		tempreg = OCTOSPI1->CR;
		tempreg &= ~(3 << 28);                	//清除FMODE原来的设置
		tempreg |= 1 << 28;                   	//设置FMODE为间接读取模式
		OCTOSPI1->CR = tempreg;                 //回写CR寄存器

		OCTOSPI1->FCR |= 1 << 1;                //清除TCF标志位
		tempreg= OCTOSPI1->IR;
		OCTOSPI1->IR = tempreg;                  	//回写AR寄存器,触发传输
		tempreg= OCTOSPI1->AR;
		OCTOSPI1->AR = tempreg;                  	//回写AR寄存器,触发传输
		while(datalen)
		{
			status = QSPI_Wait_Flag(3 << 1, 1, 0XFFFF); //等到FTF&TCF,即接收到了数据
			if(status == 0)                     //等待成功
			{
				*buf++ = *(vu8 *)data_reg;
				datalen--;
			}
			else break;
		}
		if(status == 0)
		{
			OCTOSPI1->CR |= 1 << 1;                      //终止传输
			status = QSPI_Wait_Flag(1 << 1, 1, 0XFFFF); //等待TCF,即数据传输完成
			if(status == 0)
			{
				OCTOSPI1->FCR |= 1 << 1;                 //清除TCF标志位
				status = QSPI_Wait_Flag(1 << 5, 0, 0XFFFF); //等待BUSY位清零
			}
		}
	}
    return status;
}

//QSPI发送指定长度的数据
//buf:发送数据缓冲区首地址
//datalen:要传输的数据长度
//返回值:0,正常
//    其他,错误代码
u8 QSPI_Transmit(u8 *buf, u32 datalen)
{
    u32 tempreg;
    u8 status;
    vu32 *data_reg = &OCTOSPI1->DR;
    
	OCTOSPI1->DLR = datalen - 1;             //设置数据传输长度
	
	tempreg = OCTOSPI1->CR;
	tempreg &= ~(3 << 28);                	//清除FMODE原来的设置
    tempreg |= 0 << 28;                   	//设置FMODE为间接写模式
	
    OCTOSPI1->CR = tempreg;                 //回写CR寄存器
	
    OCTOSPI1->FCR |= 1 << 1;                 //清除TCF标志位
	tempreg= OCTOSPI1->IR;
	OCTOSPI1->IR = tempreg;                  	//回写AR寄存器,触发传输
	
    while(datalen)
    {
        status = QSPI_Wait_Flag(1 << 2, 1, 0XFFFF); //等到FTF
        if(status != 0)                     //等待成功
        {
            break;
        }
        *(vu8 *)data_reg = *buf++;
        datalen--;
    }
    if(status == 0)
    {
        OCTOSPI1->CR |= 1 << 1;                      //终止传输
        status = QSPI_Wait_Flag(1 << 1, 1, 0XFFFF); //等待TCF,即数据传输完成
        if(status == 0)
        {
            OCTOSPI1->FCR |= 1 << 1;                 //清除TCF标志位
            status = QSPI_Wait_Flag(1 << 5, 0, 0XFFFF); //等待BUSY位清零
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