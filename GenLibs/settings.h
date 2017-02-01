
#ifndef GEN_SETTINGS
#define GEN_SETTINGS

#ifndef STM32F10X_MD
#define STM32F10X_MD
#endif

#include <stm32f10x.h>

#define cpuFreq 72000000 //частота МК
#define pi		3.14159265359 
#define maxSinSteps 3000 //максимальная длина массива под значения сигнала
#define transistorsMinTime  0.000000050 //минимальное время открытого транзистора, с
#define transistorsDeadTime 0.000000050 //deadTime для транзисторов, с

#define MIN_POWER_K 0.0   //минимальное значение коэффициента мощности, %
#define MAX_POWER_K 150.0 //максимальное значение коэффициента мощности, %

#define CH2_UP GPIOA->BSRR = GPIO_BSRR_BS9
#define CH2_DOWN GPIOA->BSRR = GPIO_BSRR_BR9
#define CH4_UP GPIOB->BSRR = GPIO_BSRR_BS1
#define CH4_DOWN GPIOB->BSRR = GPIO_BSRR_BR1

#define LED_GREEN_ON GPIOC->BSRR = GPIO_BSRR_BS13
#define LED_GREEN_OFF GPIOC->BSRR = GPIO_BSRR_BR13
#define LED_BLUE_ON GPIOA->BSRR = GPIO_BSRR_BS0
#define LED_BLUE_OFF GPIOA->BSRR = GPIO_BSRR_BR0
#define LED_RED_ON GPIOA->BSRR = GPIO_BSRR_BS1
#define LED_RED_OFF GPIOA->BSRR = GPIO_BSRR_BR1

#define flash_base_addr 0x0801FC00

enum
{
	EVENT_BUTTON1_CLICK = 1,
	EVENT_BUTTON2_CLICK = 2,
	EVENT_BUTTON3_CLICK = 4,
	EVENT_BUTTON4_CLICK = 8,
	EVENT_BUTTON5_CLICK = 16,
	EVENT_BUTTON6_CLICK = 32,
	EVENT_VALCODER_BUTTON_CLICK = 64,
	EVENT_VALCODER_CCW = 128,
	EVENT_VALCODER_CW  = 256		
};

#endif
