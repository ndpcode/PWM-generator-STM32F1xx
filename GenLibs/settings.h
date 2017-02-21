
#ifndef GEN_SETTINGS
#define GEN_SETTINGS

#ifndef STM32F10X_MD
#define STM32F10X_MD
#endif

#include <stm32f10x.h>

#define cpuFreq 72000000 //������� ��
#define pi		3.1415926535897932384626433832795 
#define maxSinSteps 3000 //������������ ����� ������� ��� �������� �������, maxSinSteps*4 ���� < 1/2 �� ������� RAM

#define defaultUpdateType 0           //��-��������� �������������� ����
#define defaultFreqPWM 25000          //��-��������� ������� ���
#define defaultFreqSignal 100         //��-��������� ������� �������
#define defaultPowerK 10000           //��-��������� �-� ��������, div 100
#define defaultSignalCenter 0         //��-��������� �-� �������� ������ �������, div 100
#define defaultTransistorsMinTime  50 //��-��������� ����������� ����� ��������� �����������, ��
#define defaultTransistorsDeadTime 50 //��-��������� deadTime ��� ������������, ��
#define defaultSignalType 1           //��-��������� ��� �������, ����� = 1, ����������� = 2

#define MIN_POWER_K 000000 //����������� �������� ������������ ��������, %, div 100
#define MAX_POWER_K 170000 //������������ �������� ������������ ��������, %, div 100

#define MIN_CENTER_K -100000 //����������� �������� �-�� �������� ������, div 100
#define MAX_CENTER_K 100000  //������������ �������� �-�� �������� ������, div 100

#define MIN_DEAD_TIME 0     //����������� dead time, ��
#define MAX_DEAD_TIME 10000 //������������ dead time, ��

#define MIN_PULSE_TIME 10     //����������� min duty time, ��
#define MAX_PULSE_TIME 1000   //������������ min duty time, ��

#define MENU_DRAW_FPS 50       //��, �������� ��������� ����
#define MENU_EVENTS_FPS 1000   //��, �������� ������ ������
#define MENU_TRANS_TIME 630   //����� �������� ����� �������� ����, ��

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
