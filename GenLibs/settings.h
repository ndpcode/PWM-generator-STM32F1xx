
#include "stm32f10x.h"

#ifndef CONST_BLOCK
#define CONST_BLOCK

#define cpuFreq 72000000 //частота МК
#define pi		3.14159265359 
#define maxSinSteps 3000 //максимальная длина массива под значения сигнала
#define transistorsMinTime  0.000000050 //минимальное время открытого транзистора, с
#define transistorsDeadTime 0.000000050 //deadTime для транзисторов, с

//число усреднений АЦП
#define aver_num 3000

#define CH2_UP GPIOB->BSRR = GPIO_BSRR_BS1
#define CH2_DOWN GPIOB->BSRR = GPIO_BSRR_BR1
#define CH4_UP GPIOA->BSRR = GPIO_BSRR_BS7
#define CH4_DOWN GPIOA->BSRR = GPIO_BSRR_BR7

#define LED_GREEN_ON GPIOC->BSRR = GPIO_BSRR_BS13
#define LED_GREEN_OFF GPIOC->BSRR = GPIO_BSRR_BR13
#define LED_BLUE_ON GPIOB->BSRR = GPIO_BSRR_BS4
#define LED_BLUE_OFF GPIOB->BSRR = GPIO_BSRR_BR4
#define LED_RED_ON GPIOB->BSRR = GPIO_BSRR_BS5
#define LED_RED_OFF GPIOB->BSRR = GPIO_BSRR_BR5

#define flash_base_addr 0x0801FC00
#define flash_addr1 *(__IO uint16_t*)0x0801FC00
#define flash_addr2 *(__IO uint16_t*)0x0801FC02

#endif
