
#include "stm32f10x.h"
#include "bsc_stm32_delay.h"

#ifndef cpuFreq
#define cpuFreq 72000000 //קאסעמעא ßִ׀ְ ּÊ
#endif

// DWT (Data Watchpoint Trigger)
#define DWT_CYCCNT *(volatile uint32_t *)0xE0001004
#define DWT_CONTROL *(volatile uint32_t *)0xE0001000
#define SCB_DEMCR *(volatile uint32_t *)0xE000EDFC

//--------------------------------------------------------------------//
//--------------------------------------------------------------------//
//--------------------------------------------------------------------//

//________________delayEnable____________________
void delayEnable(void)
{
	SCB_DEMCR  |= 0x01000000;
	DWT_CYCCNT  = 0; 
	DWT_CONTROL|= 1;
};

//________________delayMS____________________
void delayMS(unsigned short time)
{
	uint32_t dwt_time_start, dwt_time_size;
	dwt_time_size = time * ( cpuFreq / 1000 );
	dwt_time_start = DWT_CYCCNT;
	while ( ( DWT_CYCCNT - dwt_time_start ) < dwt_time_size );
	// Enable Timer 4
	/*RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	//Set
	TIM4->PSC = 35999; // 36 MHz div 36 000 = 1 KHz
  TIM4->ARR = time;  // number periods with length = 1ms
  TIM4->CR1 = TIM_CR1_CEN|TIM_CR1_OPM;  // start
  while ((TIM4->CR1 & TIM_CR1_CEN)!=0); // wait for end*/
};

//________________delayUS____________________
void delayUS(unsigned short time)
{
	uint32_t dwt_time_start, dwt_time_size;
	dwt_time_size = time * ( cpuFreq / 1000000 );
	dwt_time_start = DWT_CYCCNT;
	while ( ( DWT_CYCCNT - dwt_time_start ) < dwt_time_size );

	// Enable Timer 4
	/*RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	//Set
  TIM4->CR1 = TIM_CR1_OPM | TIM_CR1_URS;
	TIM4->PSC = 35; // 36 MHz div 36 = 1 MHz
	TIM4->CNT = 0;
  TIM4->ARR = time;  // number periods with length = 1us	
	dwt_timestamp = DWT_CYCCNT;
  TIM4->CR1 |= TIM_CR1_CEN;  // start
  while ((TIM4->CR1 & TIM_CR1_CEN)!=0); // wait for end
	dwt_timestamp = DWT_CYCCNT - dwt_timestamp;*/
};
