
#ifndef GEN_SETTINGS
#define GEN_SETTINGS

#ifndef STM32F10X_MD
#define STM32F10X_MD
#endif

#include <stm32f10x.h>

#define buildID "1.09"

#define cpuFreq 72000000UL //частота МК
#define pi		3.1415926535897932384626433832795 
#define maxSignSteps 3000 //максимальная длина массива под значения сигнала, maxSinSteps*4 байт < 1/2 от размера RAM

#define defaultUpdateType 0            //по-умолчанию автообновление выкл
#define defaultShowFreqType 1          //по-умолчанию показывается плавная регулировка частот
#define defaultLocLanguage 1           //по-умолчанию язык
#define defaultSignalType 1            //по-умолчанию тип сигнала, синус = 1, треугольник = 2, меандр = 3
#define defaultFreqPWM 25000           //по-умолчанию частота ШИМ
#define defaultFreqSignal 100          //по-умолчанию частота сигнала
#define defaultPowerK 10000            //по-умолчанию к-т мощности, div 10000
#define defaultSignalCenter 0          //по-умолчанию к-т смещения центра сигнала, div 100
#define defaultTransistorsMinTime  50  //по-умолчанию минимальное время открытого транзистора, нс
#define defaultTransistorsDeadTime 50  //по-умолчанию deadTime для транзисторов, нс
#define defaultTimerPrescaler 0        //по-умоланию предделитель таймера
#define defaultTimerARR 1440           //по-умолчанию auto-reload register
#define defaultTimerStepsCCR 500       //по-умолчанию capture/compare register
#define defaultTimerPrescalerPWM 0
#define defaultTimerARRPWM 720

#define signalSinus 1
#define signalTriangle 2
#define signalSquare 3
#define signalIndexMin signalSinus
#define signalIndexMax signalSquare

#define MIN_POWER_K 00.0 //минимальное значение коэффициента мощности
#define MAX_POWER_K 17.0 //максимальное значение коэффициента мощности

#define MIN_CENTER_K -50000 //минимальное значение к-та смещения центра, div 100
#define MAX_CENTER_K 50000  //максимальное значение к-та смещения центра, div 100

#define MIN_DEAD_TIME 0     //минимальный dead time, нс
#define MAX_DEAD_TIME 10000 //максимальный dead time, нс

#define MIN_PULSE_TIME 10     //минимальный min duty time, нс
#define MAX_PULSE_TIME 1000   //максимальный min duty time, нс

#define MENU_DRAW_FPS 50       //Гц, скорость отрисовки меню
#define MENU_EVENTS_FPS 1000   //Гц, скорость опроса кнопок
#define MENU_TRANS_TIME 630   //время перехода между пунктами меню, мс

#define ShowFreqTypeNormal 1
#define ShowFreqTypeActualFreq 2
#define ShowFreqTypeDirectControl 3

#define CH1_DOWN GPIOA->BSRR = GPIO_BSRR_BR8
#define CH3_DOWN GPIOA->BSRR = GPIO_BSRR_BR10
#define CH2_UP GPIOA->BSRR = GPIO_BSRR_BS7
#define CH2_DOWN GPIOA->BSRR = GPIO_BSRR_BR7
#define CH4_UP GPIOB->BSRR = GPIO_BSRR_BS1
#define CH4_DOWN GPIOB->BSRR = GPIO_BSRR_BR1

#define CH3_SET GPIOA->CRH &= ~( GPIO_CRH_CNF8 | GPIO_CRH_MODE8 );\
                GPIOA->CRH |= ( GPIO_CRH_CNF8_1 | GPIO_CRH_MODE8 )
#define CH3_RESET GPIOA->CRH &= ~( GPIO_CRH_CNF8 | GPIO_CRH_MODE8 );\
                  GPIOA->CRH |= ( GPIO_CRH_CNF8_0 )
#define CH1_SET GPIOA->CRH &= ~( GPIO_CRH_CNF10 | GPIO_CRH_MODE10 );\
                GPIOA->CRH |= ( GPIO_CRH_CNF10_1 | GPIO_CRH_MODE10 )
#define CH1_RESET GPIOA->CRH &= ~( GPIO_CRH_CNF10 | GPIO_CRH_MODE10 );\
                  GPIOA->CRH |= ( GPIO_CRH_CNF10_0 )
#define CH2_SET GPIOA->CRL &= ~( GPIO_CRL_CNF7 | GPIO_CRL_MODE7 );\
                GPIOA->CRL |= ( GPIO_CRL_MODE7 )
#define CH2_RESET GPIOA->CRL &= ~( GPIO_CRL_CNF7 | GPIO_CRL_MODE7 );\
                  GPIOA->CRL |= ( GPIO_CRL_CNF7_0 )
#define CH4_SET GPIOB->CRL &= ~( GPIO_CRL_CNF1 | GPIO_CRL_MODE1 );\
                GPIOB->CRL |= ( GPIO_CRL_MODE1 )
#define CH4_RESET GPIOB->CRL &= ~( GPIO_CRL_CNF1 | GPIO_CRL_MODE1 );\
                  GPIOB->CRL |= ( GPIO_CRL_CNF1_0 )

#define LED_GREEN_ON GPIOC->BSRR = GPIO_BSRR_BS13
#define LED_GREEN_OFF GPIOC->BSRR = GPIO_BSRR_BR13
#define LED_BLUE_ON GPIOA->BSRR = GPIO_BSRR_BS0
#define LED_BLUE_OFF GPIOA->BSRR = GPIO_BSRR_BR0
#define LED_RED_ON GPIOA->BSRR = GPIO_BSRR_BS1
#define LED_RED_OFF GPIOA->BSRR = GPIO_BSRR_BR1

#define flash_base_addr 0x0801FC00

#endif
