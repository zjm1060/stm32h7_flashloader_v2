/*
 * sys.h
 *
 *  Created on: Apr 14, 2022
 *      Author: zjm09
 */

#ifndef SRC_SYS_H_
#define SRC_SYS_H_

#define GPIO_MODE_IN        0       //��ͨ����ģʽ
#define GPIO_MODE_OUT       1       //��ͨ���ģʽ
#define GPIO_MODE_AF        2       //AF����ģʽ
#define GPIO_MODE_AIN       3       //ģ������ģʽ

#define GPIO_SPEED_LOW      0       //GPIO�ٶ�(����,12M)
#define GPIO_SPEED_MID      1       //GPIO�ٶ�(����,60M)
#define GPIO_SPEED_FAST     2       //GPIO�ٶ�(����,85M)
#define GPIO_SPEED_HIGH     3       //GPIO�ٶ�(����,100M)

#define GPIO_PUPD_NONE      0       //����������
#define GPIO_PUPD_PU        1       //����
#define GPIO_PUPD_PD        2       //����
#define GPIO_PUPD_RES       3       //����

#define GPIO_OTYPE_PP       0       //�������
#define GPIO_OTYPE_OD       1       //��©���

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

void GPIO_AF_Set(GPIO_TypeDef *GPIOx, uint8_t BITx, uint8_t AFx);
void GPIO_Set(GPIO_TypeDef *GPIOx, uint32_t BITx, uint32_t MODE, uint32_t OTYPE, uint32_t OSPEED, uint32_t PUPD);

#endif /* SRC_SYS_H_ */
