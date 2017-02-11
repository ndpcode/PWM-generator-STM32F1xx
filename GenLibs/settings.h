
#ifndef GEN_SETTINGS
#define GEN_SETTINGS

#ifndef STM32F10X_MD
#define STM32F10X_MD
#endif

#include <stm32f10x.h>

#define cpuFreq 72000000 //������� ��
#define pi		3.14159265359 
#define maxSinSteps 3000 //������������ ����� ������� ��� �������� �������
#define transistorsMinTime  0.000000050 //����������� ����� ��������� �����������, �
#define transistorsDeadTime 0.000000050 //deadTime ��� ������������, �

#define MIN_POWER_K 0.0   //����������� �������� ������������ ��������, %
#define MAX_POWER_K 150.0 //������������ �������� ������������ ��������, %

#define MENU_DRAW_FPS 50       //��, �������� ��������� ����
#define MENU_EVENTS_FPS 1000   //��, �������� ������ ������
#define MENU_TRANS_TIME 630   //����� �������� ����� �������� ����, ��

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

#endif
