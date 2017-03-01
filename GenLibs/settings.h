
#ifndef GEN_SETTINGS
#define GEN_SETTINGS

#ifndef STM32F10X_MD
#define STM32F10X_MD
#endif

#include <stm32f10x.h>

#define cpuFreq 72000000UL //������� ��
#define pi		3.1415926535897932384626433832795 
#define maxSinSteps 3000 //������������ ����� ������� ��� �������� �������, maxSinSteps*4 ���� < 1/2 �� ������� RAM

#define defaultUpdateType 0           //��-��������� �������������� ����
#define defaultShowRealFreqType 0     //��-��������� �� ���������� ������ �������� �������, ������������ ������� �����������
#define defaultFreqPWM 25000          //��-��������� ������� ���
#define defaultFreqSignal 100         //��-��������� ������� �������
#define defaultPowerK 10000           //��-��������� �-� ��������, div 100
#define defaultSignalCenter 0         //��-��������� �-� �������� ������ �������, div 100
#define defaultTransistorsMinTime  50 //��-��������� ����������� ����� ��������� �����������, ��
#define defaultTransistorsDeadTime 50 //��-��������� deadTime ��� ������������, ��
#define defaultSignalType 1           //��-��������� ��� �������, ����� = 1, ����������� = 2, ������ = 3

#define MIN_POWER_K 000000 //����������� �������� ������������ ��������, %, div 100
#define MAX_POWER_K 170000 //������������ �������� ������������ ��������, %, div 100

#define MIN_CENTER_K -50000 //����������� �������� �-�� �������� ������, div 100
#define MAX_CENTER_K 50000  //������������ �������� �-�� �������� ������, div 100

#define MIN_DEAD_TIME 0     //����������� dead time, ��
#define MAX_DEAD_TIME 10000 //������������ dead time, ��

#define MIN_PULSE_TIME 10     //����������� min duty time, ��
#define MAX_PULSE_TIME 1000   //������������ min duty time, ��

#define MENU_DRAW_FPS 50       //��, �������� ��������� ����
#define MENU_EVENTS_FPS 1000   //��, �������� ������ ������
#define MENU_TRANS_TIME 630   //����� �������� ����� �������� ����, ��

#define CH1_DOWN GPIOA->BSRR = GPIO_BSRR_BR8
#define CH3_DOWN GPIOA->BSRR = GPIO_BSRR_BR10
#define CH2_UP GPIOA->BSRR = GPIO_BSRR_BS7
#define CH2_DOWN GPIOA->BSRR = GPIO_BSRR_BR7
#define CH4_UP GPIOB->BSRR = GPIO_BSRR_BS1
#define CH4_DOWN GPIOB->BSRR = GPIO_BSRR_BR1

#define CH1_SET GPIOA->CRH &= ~( GPIO_CRH_CNF8 | GPIO_CRH_MODE8 );\
                GPIOA->CRH |= ( GPIO_CRH_CNF8_1 | GPIO_CRH_MODE8 )
#define CH1_RESET GPIOA->CRH &= ~( GPIO_CRH_CNF8 | GPIO_CRH_MODE8 );\
                  GPIOA->CRH |= ( GPIO_CRH_CNF8_0 )
#define CH3_SET GPIOA->CRH &= ~( GPIO_CRH_CNF10 | GPIO_CRH_MODE10 );\
                GPIOA->CRH |= ( GPIO_CRH_CNF10_1 | GPIO_CRH_MODE10 )
#define CH3_RESET GPIOA->CRH &= ~( GPIO_CRH_CNF10 | GPIO_CRH_MODE10 );\
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
