#include "stm32f10x.h"
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


void gen_init_ports(void)
{
	//PORT A, B, C power on
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN; // port A,B,C power on
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; //Alternate Function I/O clock enable
	//PB 4 - free! Full SWJ (JTAG-DP + SW-DP) but without JNTRST
	AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG_DISABLE;
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_NOJNTRST;
	//set PORTA
	GPIOA->CRL = MODE_INPUT_ANALOG(0, L) | //PA 0 - ADC "F2 точно"
               MODE_INPUT_ANALOG(1, L) | //PA 1 - ADC "F2 скважность"
	             MODE_INPUT_ANALOG(2, L) | //PA 2 - ADC "Сдвиг фазы"
	             MODE_INPUT_ANALOG(3, L) | //PA 3 - ADC "F2 грубо"
	             MODE_INPUT_ANALOG(4, L) | //PA 4 - ADC "F1 скважность"
	             MODE_INPUT_ANALOG(5, L) | //PA 5 - ADC "F1 точно"
	             MODE_INPUT_ANALOG(6, L) | //PA 6 - ADC "F1 грубо"
	             MODE_OUTPUT_PP(7, L, 50); //PA 7 - TIM3_CH2 (II)
	GPIOA->CRH = MODE_OUTPUT_ALT_PP(8, H, 50) | //PA 8 - TIM1_CH1 (III)
	             MODE_INPUT_FLOATING(9, H) | //PA 9 - TIM1_CH2 (II)
	             MODE_OUTPUT_ALT_PP(10, H, 50); //PA 10 - TIM1_CH3 (I)
	
	//set PORTB
	GPIOB->CRL = MODE_INPUT_FLOATING(0, L) | //PB 0 - TIM3_CH3 (I)
	             MODE_OUTPUT_PP(1, L, 50) |  //PB 1 - TIM3_CH4 (IV)
	             MODE_OUTPUT_PP(4, L, 2) |      //PB 4 - LED 1
	             MODE_OUTPUT_PP(5, L, 2) ;      //PB 5 - LED 2 
	GPIOB->CRH = MODE_INPUT_PUPD(10, H) |       //PB 10 - Button 1
	             MODE_INPUT_PUPD(11, H) |       //PB 11 - Button 2
							 MODE_INPUT_PUPD(12, H) |       //PB 12 - Button 3
							 MODE_INPUT_PUPD(13, H) |       //PB 13 - Button 4
							 MODE_INPUT_PUPD(14, H) |       //PB 14 - Button 5
							 MODE_INPUT_PUPD(15, H) ;       //PB 15 - Button 6
	//pull up buttons
  GPIOB->BSRR = GPIO_BSRR_BS10 | GPIO_BSRR_BS11 | GPIO_BSRR_BS12 |
	              GPIO_BSRR_BS13 | GPIO_BSRR_BS14 |  GPIO_BSRR_BS15;
	
	//set PORTC
	GPIOC->CRL = 0;
	GPIOC->CRH = MODE_OUTPUT_PP(13, H, 2);      //PC 13 - LED 3 
}
