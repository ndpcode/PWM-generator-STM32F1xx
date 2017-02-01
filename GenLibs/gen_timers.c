
#include "settings.h"
#include "gen_timers.h"
#include <math.h>
#include <string.h>

uint16_t accuracySignalA, accuracySignalB; //точность ШИМ, сколько периодов генератора МК укладывается в период ШИМ
uint16_t pwmSignalStepsA, pwmSignalStepsB; //число периодов ШИМ в одном полупериоде сигнала
uint16_t pwmSignalArrayA[maxSinSteps];  //массив А под отсчеты сигнала
uint16_t pwmSignalArrayB[maxSinSteps];  //массив B под отсчеты сигнала    
int8_t signalH, signalL;

struct timerEventsBase
{
	unsigned currentArray : 1; //текущий используемый массив с отсчетами сигнала ( A (0) или B (1) )
	unsigned mayChangeArray : 1; //флаг возможности смены массива с отсчетами
	unsigned signalEnabled : 1;
} timerEvents;

uint8_t GenInitTimers(void)
{
	memset( &timerEvents, 0, sizeof(timerEvents) );
  memset( pwmSignalArrayA, 0, sizeof(uint16_t) * maxSinSteps );	
	memset( pwmSignalArrayB, 0, sizeof(uint16_t) * maxSinSteps );
	signalH = 1;
	signalL = 0;
	
	//вкл Timer1
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	//инициация Timer1
	TIM1->CR1 &= ~TIM_CR1_CEN;	
  TIM1->CR1 = TIM_CR1_ARPE; //| TIM_CR1_CMS;
  //TIM1->RCR = 0;
  TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC3E;
	TIM1->CCMR1 = TIM_CCMR1_OC1FE | TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_1 ;
  TIM1->CCMR2 = TIM_CCMR2_OC3FE | TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M_1 ;
	TIM1->DIER |= TIM_DIER_CC1DE | TIM_DIER_CC3DE;
	TIM1->BDTR = TIM_BDTR_MOE;
	TIM1->PSC = 0;
	TIM1->ARR = 1000;
	TIM1->CCR1 = 0;
	TIM1->CCR3 = 0;
	TIM1->CR1 |= TIM_CR1_CEN;
	
	//временное значение
	pwmSignalStepsA = 10;
	//вкл DMA, DMA->Timer1
  RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA1_Channel2->CPAR = (uint32_t) &TIM1->CCR1;
	DMA1_Channel6->CPAR = (uint32_t) &TIM1->CCR3;
	DMA1_Channel2->CMAR = (uint32_t) &pwmSignalArrayA[0];
	DMA1_Channel6->CMAR = (uint32_t) &pwmSignalArrayA[0];
	DMA1_Channel2->CNDTR = pwmSignalStepsA;
	DMA1_Channel6->CNDTR = pwmSignalStepsA;
	DMA1_Channel2->CCR = DMA_CCR2_PL | DMA_CCR2_MINC | DMA_CCR2_CIRC | DMA_CCR2_DIR | DMA_CCR2_MSIZE_0 | DMA_CCR2_PSIZE_0;
	DMA1_Channel6->CCR = DMA_CCR2_PL | DMA_CCR6_MINC | DMA_CCR6_CIRC | DMA_CCR6_DIR | DMA_CCR6_MSIZE_0 | DMA_CCR6_PSIZE_0;
	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
	NVIC_EnableIRQ(DMA1_Channel2_IRQn);
	NVIC_EnableIRQ(DMA1_Channel6_IRQn);
	DMA1_Channel6->CCR &= 0xFFFFFFFE;
  DMA1_Channel2->CCR |= DMA_CCR2_EN;

  return 1;
}

void calculateSinArray( uint16_t *_sin_array, uint16_t _arr_size, uint16_t _acc_sin, double _power_k )
{
	uint16_t i = 0;
	uint16_t transistorsMinStep = transistorsMinTime * cpuFreq;	
	uint16_t transistorsMaxStep = _acc_sin - transistorsDeadTime * cpuFreq;
	for (i = 0; i < _arr_size; i++){
		_sin_array[i] = (double)sin( (i+1)*pi / _arr_size )*_acc_sin*_power_k + transistorsMinStep;
		//контроль для DeadTime отдельно, для прозрачности
		if ( _sin_array[i] > transistorsMaxStep ) _sin_array[i] = transistorsMaxStep;
	};
	//крайние значения принудительно к 0
	//_sin_array[0] = 0;
	//_sin_array[_arr_size-1] = 0;
}

void createSinArray(uint32_t _freq_pwm, uint32_t _freq_sin, double _power_k)
{	
	if ( !timerEvents.currentArray ) //если текущий буфер - А
	{
		accuracySignalB = cpuFreq / ( _freq_pwm );
		pwmSignalStepsB = _freq_pwm / ( 2*_freq_sin );
		calculateSinArray(pwmSignalArrayB, pwmSignalStepsB, accuracySignalB, _power_k);
	} else
	{ //теущий буфер - B
		accuracySignalA = cpuFreq / ( _freq_pwm );
		pwmSignalStepsA = _freq_pwm / ( 2*_freq_sin );
		calculateSinArray(pwmSignalArrayA, pwmSignalStepsA, accuracySignalA, _power_k);
	};	
};

//________________startSignal____________________
uint8_t updateSignal(uint32_t _freq_pwm, uint32_t _freq_signal, double _power_k, uint8_t _signal_type)
{
	//принудительно убираем флаг
	timerEvents.mayChangeArray = 0;
	//выбор сигнала и заполнение массива
	switch ( _signal_type )
	{
		case 0: //sinus
			createSinArray(_freq_pwm, _freq_signal, _power_k);			
		break;
	};	
	//подготовка флагов к плавному переключению сигнала
	//по прерыванию DMA будет изменен буффер сигнала
	timerEvents.mayChangeArray = 1;
	
	return 1;
};

void DMA_Interrupt_Change_Signal(void)
{
	//принудительно выкл DMA
	DMA1_Channel2->CCR &= 0xFFFFFFFE;
	DMA1_Channel6->CCR &= 0xFFFFFFFE;
	//измен буфер
	if ( !timerEvents.currentArray ) //текущий буфер - А
	{
		DMA1_Channel2->CMAR = (uint32_t) &pwmSignalArrayB[0];
		DMA1_Channel6->CMAR = (uint32_t) &pwmSignalArrayB[0];
	  DMA1_Channel2->CNDTR = pwmSignalStepsB;
	  DMA1_Channel6->CNDTR = pwmSignalStepsB;
	  TIM1->ARR = accuracySignalB;
	  TIM1->CCR1 = 0;
	  TIM1->CCR3 = 0;
		//переключение флага на B
		timerEvents.currentArray = 1;
    timerEvents.signalEnabled = 1;		
	} else
	{
		//текущий буфер - B
		DMA1_Channel2->CMAR = (uint32_t) &pwmSignalArrayA[0];
	  DMA1_Channel6->CMAR = (uint32_t) &pwmSignalArrayA[0];
	  DMA1_Channel2->CNDTR = pwmSignalStepsA;
	  DMA1_Channel6->CNDTR = pwmSignalStepsA;
	  TIM1->ARR = accuracySignalA;
	  TIM1->CCR1 = 0;
	  TIM1->CCR3 = 0;
		//переключение флага на A
		timerEvents.currentArray = 0;
		timerEvents.signalEnabled = 1;
	};
	// выкл
	timerEvents.mayChangeArray = 0;
}

void DMA_Interrupt_Change_Period(void)
{
	if(signalH == 1) {
		//выкл 2
		CH2_DOWN;
		//выкл 1
		TIM1->CCMR1 &= ~TIM_CCMR1_OC1M_2;
		//меняем сигнал
		if ( timerEvents.mayChangeArray ) DMA_Interrupt_Change_Signal();
		//вкл 3
		TIM1->CCMR2 |=  TIM_CCMR2_OC3M_2;
		//вкл 4
		if ( timerEvents.signalEnabled ) CH4_UP;
		DMA1_Channel2->CCR &= 0xFFFFFFFE;
		DMA1_Channel6->CCR |= DMA_CCR6_EN;
	}
	else  {
		//выкл 4
		CH4_DOWN;
		//выкл 3
		TIM1->CCMR2 &= ~TIM_CCMR2_OC3M_2;
		//меняем сигнал
		if ( timerEvents.mayChangeArray ) DMA_Interrupt_Change_Signal();
		//вкл 1
		TIM1->CCMR1 |=  TIM_CCMR1_OC1M_2;		
		//вкл 2
		if ( timerEvents.signalEnabled ) CH2_UP;
		DMA1_Channel6->CCR &= 0xFFFFFFFE;
		DMA1_Channel2->CCR |= DMA_CCR2_EN;
	};
	signalH = 1 - signalH;
}

void DMA1_Channel2_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA1_IT_TC2);
  DMA_Interrupt_Change_Period();
};

void DMA1_Channel6_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA1_IT_TC6);
  DMA_Interrupt_Change_Period();
};
