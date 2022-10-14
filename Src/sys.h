/*
 * sys.h
 *
 *  Created on: Apr 14, 2022
 *      Author: zjm09
 */

#ifndef __SYS_H
#define __SYS_H

#include "stm32h7b0xx.h"
#include "stdio.h"

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef volatile uint32_t  vu32;
typedef volatile uint16_t vu16;
typedef volatile uint8_t  vu8;

#define GPIO_A              0
#define GPIO_B              1
#define GPIO_C              2
#define GPIO_D              3
#define GPIO_E              4
#define GPIO_F              5
#define GPIO_G              6
#define GPIO_H              7
#define GPIO_I              8
#define GPIO_J              9
#define GPIO_K              10

#define FTIR                1       //下降沿触发
#define RTIR                2       //上升沿触发

#define GPIO_MODE_IN        0       //普通输入模式
#define GPIO_MODE_OUT       1       //普通输出模式
#define GPIO_MODE_AF        2       //AF功能模式
#define GPIO_MODE_AIN       3       //模拟输入模式

#define GPIO_SPEED_LOW      0       //GPIO速度(低速,12M)
#define GPIO_SPEED_MID      1       //GPIO速度(中速,60M)
#define GPIO_SPEED_FAST     2       //GPIO速度(快速,85M)
#define GPIO_SPEED_HIGH     3       //GPIO速度(高速,100M)  

#define GPIO_PUPD_NONE      0       //不带上下拉
#define GPIO_PUPD_PU        1       //上拉
#define GPIO_PUPD_PD        2       //下拉
#define GPIO_PUPD_RES       3       //保留 

#define GPIO_OTYPE_PP       0       //推挽输出
#define GPIO_OTYPE_OD       1       //开漏输出 

#define PIN0                1<<0
#define PIN1                1<<1
#define PIN2                1<<2
#define PIN3                1<<3
#define PIN4                1<<4
#define PIN5                1<<5
#define PIN6                1<<6
#define PIN7                1<<7
#define PIN8                1<<8
#define PIN9                1<<9
#define PIN10               1<<10
#define PIN11               1<<11
#define PIN12               1<<12
#define PIN13               1<<13
#define PIN14               1<<14
#define PIN15               1<<15

void Stm32_Clock_Init(u32 plln, u32 pllm, u32 pllp, u32 pllq);
u8 GPIO_Pin_Get(GPIO_TypeDef *GPIOx, u16 pinx);
void GPIO_Pin_Set(GPIO_TypeDef *GPIOx, u16 pinx, u8 status);
void GPIO_Pin_Togle(GPIO_TypeDef *GPIOx, u16 pinx);
void GPIO_Set(GPIO_TypeDef *GPIOx, u32 BITx, u32 MODE, u32 OTYPE, u32 OSPEED, u32 PUPD);
void GPIO_AF_Set(GPIO_TypeDef *GPIOx, u8 BITx, u8 AFx);
void uart_init(u32 pclk, u32 bound);

#endif /* SRC_SYS_H_ */
