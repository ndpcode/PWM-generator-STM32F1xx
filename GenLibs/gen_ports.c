
#include "settings.h"
#include "gen_ports.h"

#define MODE_INPUT_ANALOG(_pin_number, _lh) 0
#define MODE_INPUT_FLOATING(_pin_number, _lh) ( GPIO_CR##_lh##_CNF##_pin_number##_0 )
#define MODE_INPUT_PUPD(_pin_number, _lh) ( GPIO_CR##_lh##_CNF##_pin_number##_1 )
#define MODE_10MHZ(_pin_number, _lh) ( GPIO_CR##_lh##_MODE##_pin_number##_0 )
#define MODE_2MHZ(_pin_number, _lh) ( GPIO_CR##_lh##_MODE##_pin_number##_1 )
#define MODE_50MHZ(_pin_number, _lh) ( GPIO_CR##_lh##_MODE##_pin_number )
#define MODE_OUTPUT_PP(_pin_number, _lh, _speed_mhz) MODE_##_speed_mhz##MHZ(_pin_number, _lh)
#define MODE_OUTPUT_OD(_pin_number, _lh, _speed_mhz) ( GPIO_CR##_lh##_CNF##_pin_number##_0 | MODE_##_speed_mhz##MHZ(_pin_number, _lh) )
#define MODE_OUTPUT_ALT_PP(_pin_number, _lh, _speed_mhz) ( GPIO_CR##_lh##_CNF##_pin_number##_1 | MODE_##_speed_mhz##MHZ(_pin_number, _lh) )
#define MODE_OUTPUT_ALT_OD(_pin_number, _lh, _speed_mhz) ( GPIO_CR##_lh##_CNF##_pin_number | MODE_##_speed_mhz##MHZ(_pin_number, _lh) )


uint8_t GenInitPorts(void)
{
	//PORT A, B, C power on
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN; // port A,B,C power on
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; //Alternate Function I/O clock enable
	//PB 4 and PB 3 - free! JTAG-DP Disabled and SW-DP Enabled
	AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG;
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
	
	//set PORTA
	GPIOA->CRL = MODE_OUTPUT_PP(0, L, 2) | //PA 0 - LED BLUE
               MODE_OUTPUT_PP(1, L, 2) | //PA 1 - LED RED
	             MODE_INPUT_PUPD(2, L) | //PA 2 - Valcoder CCW
	             MODE_INPUT_PUPD(3, L) | //PA 3 - Valcoder Button
	             MODE_INPUT_PUPD(4, L) | //PA 4 - Valcoder CW
	             MODE_INPUT_FLOATING(5, L) | //PA 5 - N/A
	             MODE_INPUT_FLOATING(6, L) | //PA 6 - N/A
	             MODE_OUTPUT_PP(7, L, 50) ; //PA 9 - Signal OUT 2 - OUT_PP
	GPIOA->CRH = MODE_OUTPUT_ALT_PP(8, H, 50) | //PA 8 - Signal OUT 3 - TIM1_CH1
	             MODE_INPUT_FLOATING(9, H) | //PA 9 - N/A
	             MODE_OUTPUT_ALT_PP(10, H, 50) | //PA 10 - Signal OUT 1 - TIM1_CH3
	             MODE_INPUT_FLOATING(11, H) | //PA 11 - N/A
	             MODE_INPUT_FLOATING(12, H) | //PA 12 - N/A
	             MODE_INPUT_FLOATING(13, H) | //PA 13 - N/A
							 MODE_INPUT_FLOATING(14, H) | //PA 14 - N/A
			         MODE_INPUT_FLOATING(15, H) ; //PA 15 - N/A							 
	//pull-up buttons and valcoder
	GPIOA->BSRR = GPIO_BSRR_BS2 | GPIO_BSRR_BS3 | GPIO_BSRR_BS4;
	
	//set PORTB
	GPIOB->CRL = MODE_INPUT_FLOATING(0, L) | //PB 0 - N/A
               MODE_OUTPUT_PP(1, L, 50) | //PB 1 - Signal OUT 4 - OUT_PP
	             MODE_INPUT_FLOATING(2, L) | //PB 2 - N/A
	             MODE_OUTPUT_OD(3, L, 10) | //PB 3 - Display A0
	             MODE_OUTPUT_OD(4, L, 10) | //PB 4 - Display DB4
	             MODE_OUTPUT_OD(5, L, 10) | //PB 5 - Display DB5
	             MODE_OUTPUT_OD(6, L, 10) | //PB 6 - Display DB6
	             MODE_OUTPUT_OD(7, L, 10) ; //PB 7 - Display DB7
	GPIOB->CRH = MODE_OUTPUT_OD(8, H, 10) | //PB 8 - Display RW
	             MODE_OUTPUT_OD(9, H, 10) | //PB 9 - Display E
	             MODE_INPUT_PUPD(10, H) | //PB 10 - Button 1
	             MODE_INPUT_PUPD(11, H) | //PB 11 - Button 2
	             MODE_INPUT_PUPD(12, H) | //PB 12 - Button 3
	             MODE_INPUT_PUPD(13, H) | //PB 13 - Button 4
							 MODE_INPUT_PUPD(14, H) | //PB 14 - Button 5
			         MODE_INPUT_PUPD(15, H) ; //PB 15 - Button 6
	//pull-up buttons
	GPIOB->BSRR = GPIO_BSRR_BS10 | GPIO_BSRR_BS11 | GPIO_BSRR_BS12 |
	              GPIO_BSRR_BS13 | GPIO_BSRR_BS14 | GPIO_BSRR_BS15;
	
	//set PORTC
	GPIOC->CRL = MODE_INPUT_FLOATING(0, L) | //PC 0 - N/A
               MODE_INPUT_FLOATING(1, L) | //PC 1 - N/A
	             MODE_INPUT_FLOATING(2, L) | //PC 2 - N/A
	             MODE_INPUT_FLOATING(3, L) | //PC 3 - N/A
	             MODE_INPUT_FLOATING(4, L) | //PC 4 - N/A
	             MODE_INPUT_FLOATING(5, L) | //PC 5 - N/A
	             MODE_INPUT_FLOATING(6, L) | //PC 6 - N/A
	             MODE_INPUT_FLOATING(7, L) ; //PC 7 - N/A
	GPIOC->CRH = MODE_INPUT_FLOATING(8, H) | //PC 8 - N/A
	             MODE_INPUT_FLOATING(9, H) | //PC 9 - N/A
	             MODE_INPUT_FLOATING(10, H) | //PC 10 - N/A
	             MODE_INPUT_FLOATING(11, H) | //PC 11 - N/A
	             MODE_INPUT_FLOATING(12, H) | //PC 12 - N/A
	             MODE_OUTPUT_PP(13, H, 2) | //PC 13 - LED GREEN
							 MODE_INPUT_FLOATING(14, H) | //PC 14 - N/A
			         MODE_INPUT_FLOATING(15, H) ; //PC 15 - N/A 
	
	return 1;
}
