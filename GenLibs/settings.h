
#ifndef GEN_SETTINGS
#define GEN_SETTINGS

#ifndef STM32F10X_MD
#define STM32F10X_MD
#endif

#include <stm32f10x.h>

#define cpuFreq 72000000 //частота МК
#define pi		3.1415926535897932384626433832795 
#define maxSinSteps 3000 //максимальная длина массива под значения сигнала, maxSinSteps*4 байт < 1/2 от размера RAM

#define defaultUpdateType 0           //по-умолчанию автообновление выкл
#define defaultFreqPWM 25000          //по-умолчанию частота ШИМ
#define defaultFreqSignal 100         //по-умолчанию частота сигнала
#define defaultPowerK 100.00          //по-умолчанию к-т мощности
#define defaultSignalCenter 0.00      //по-умолчанию к-т смещения центра сигнала
#define defaultTransistorsMinTime  50 //по-умолчанию минимальное время открытого транзистора, нс
#define defaultTransistorsDeadTime 50 //по-умолчанию deadTime для транзисторов, нс
#define defaultSignalType 1           //по-умолчанию тип сигнала, синус = 1, треугольник = 2

#define MIN_POWER_K 0000.0 //минимальное значение коэффициента мощности, %
#define MAX_POWER_K 1700.0 //максимальное значение коэффициента мощности, %

#define MIN_CENTER_K -1000.00 //минимальное значение к-та смещения центра
#define MAX_CENTER_K 1000.00  //максимальное значение к-та смещения центра

#define MENU_DRAW_FPS 50       //Гц, скорость отрисовки меню
#define MENU_EVENTS_FPS 1000   //Гц, скорость опроса кнопок
#define MENU_TRANS_TIME 630   //время перехода между пунктами меню, мс

#define CH2_UP GPIOA->BSRR = GPIO_BSRR_BS7
#define CH2_DOWN GPIOA->BSRR = GPIO_BSRR_BR7
#define CH4_UP GPIOB->BSRR = GPIO_BSRR_BS1
#define CH4_DOWN GPIOB->BSRR = GPIO_BSRR_BR1

#define LED_GREEN_ON GPIOC->BSRR = GPIO_BSRR_BS13
#define LED_GREEN_OFF GPIOC->BSRR = GPIO_BSRR_BR13
#define LED_BLUE_ON GPIOA->BSRR = GPIO_BSRR_BS0
#define LED_BLUE_OFF GPIOA->BSRR = GPIO_BSRR_BR0
#define LED_RED_ON GPIOA->BSRR = GPIO_BSRR_BS1
#define LED_RED_OFF GPIOA->BSRR = GPIO_BSRR_BR1

#define flash_base_addr 0x0801FC00

#endif
