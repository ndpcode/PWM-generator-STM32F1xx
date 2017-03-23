
#include "settings.h"
#include "gen_timers.h"
#include <math.h>
#include <string.h>
#include <limits.h>

#define TIMER_INIT_AS_DMA 1
#define TIMER_INIT_AS_PWM 2

#define TIMER_CURRENT_A 0
#define TIMER_CURRENT_B 1

uint16_t timer1Prescaler;
uint16_t accuracySignalA, accuracySignalB; //точность ШИМ, сколько периодов генератора МК укладывается в период ШИМ
uint16_t pwmSignalStepsA, pwmSignalStepsB; //число периодов ШИМ в одном полупериоде сигнала
uint16_t pwmSignalArrayA[maxSignSteps];  //массив А под отсчеты сигнала
uint16_t pwmSignalArrayB[maxSignSteps];  //массив B под отсчеты сигнала    
int8_t signalH;

struct timerEventsBase
{
	unsigned currentArray : 1;   //текущий используемый массив с отсчетами сигнала ( A (0) или B (1) )
	unsigned mayChangeArray : 1; //флаг возможности смены массива с отсчетами
	unsigned signalEnabled : 1;  //разрешение сигнала
	unsigned timerInitType : 2;  //1 - DMA to Timer1, 2 - Timer as PWM
} timerEvents;

uint8_t Timer1Reset(void)
{
	TIM1->CR1 &= ~TIM_CR1_CEN;	
	
	TIM1->CR1 = 0x0000;
	TIM1->CR2 = 0x0000;
	TIM1->SMCR = 0x0000;
	TIM1->DIER = 0x0000;
	TIM1->SR = 0x0000;
	TIM1->EGR = 0x0000;
	TIM1->CCMR1 = 0x0000;
	TIM1->CCMR2 = 0x0000;
	TIM1->CCER = 0x0000;
	TIM1->CNT = 0x0000;
	TIM1->PSC = 0x0000;
	TIM1->ARR = 0x0000;
	TIM1->RCR = 0x0000;
	TIM1->CCR1 = 0x0000;
	TIM1->CCR2 = 0x0000;
	TIM1->CCR3 = 0x0000;
	TIM1->CCR4 = 0x0000;
	TIM1->BDTR = 0x0000;
	TIM1->DCR = 0x0000;
	TIM1->DMAR = 0x0000;
	
	timer1Prescaler = 0;
	
	CH1_DOWN;
	CH3_DOWN;
	
	return 1;
}

uint8_t DMA1Reset(void)
{	
	DMA1_Channel2->CCR &= 0xFFFFFFFE;
	DMA1_Channel6->CCR &= 0xFFFFFFFE;
	NVIC_DisableIRQ(DMA1_Channel2_IRQn);
	NVIC_DisableIRQ(DMA1_Channel6_IRQn);  
	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, DISABLE);
	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, DISABLE);	
	DMA1_Channel2->CCR = 0x00000000;
	DMA1_Channel6->CCR = 0x00000000;
	DMA1_Channel2->CNDTR = 0x00000000;
	DMA1_Channel6->CNDTR = 0x00000000;	
	DMA1_Channel2->CPAR = 0x00000000;
	DMA1_Channel6->CPAR = 0x00000000;
	DMA1_Channel2->CMAR = 0x00000000;
	DMA1_Channel6->CMAR = 0x00000000;	
	
	return 1;
}

uint8_t GenSignalEmulStop(void)
{
	//flags off
	timerEvents.mayChangeArray = 0;
	timerEvents.signalEnabled = 0;
	//ports off
	CH1_RESET;
	CH3_RESET;
	CH2_RESET;
	CH4_RESET;
	CH1_DOWN;
	CH2_DOWN;
	CH3_DOWN;
	CH4_DOWN;
	//DMA off
	DMA1Reset();
	Timer1Reset();
	
	return 1;
}

uint8_t GenSetPorts(void)
{
	//down
	CH1_DOWN;
	CH2_DOWN;
	CH3_DOWN;
	CH4_DOWN;
	//set
	CH1_SET;
	CH3_SET;
	CH2_SET;
	CH4_SET;
	//down
	CH1_DOWN;
	CH2_DOWN;
	CH3_DOWN;
	CH4_DOWN;
	
	return 1;
}

uint8_t GenInitTimer1ForSignalEmul(void)
{
	GenSignalEmulStop();
	GenSetPorts();
	
	memset( &timerEvents, 0, sizeof(timerEvents) );
  memset( pwmSignalArrayA, 0, sizeof(uint16_t) * maxSignSteps );	
	memset( pwmSignalArrayB, 0, sizeof(uint16_t) * maxSignSteps );
	signalH = 1;
	
	//вкл Timer1
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	//инициализация Timer1
	TIM1->CR1 &= ~TIM_CR1_CEN;	
  //TIM1->RCR = 0;
  TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC3E;
	TIM1->CCMR1 = TIM_CCMR1_OC1FE | TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_1 ;
  TIM1->CCMR2 = TIM_CCMR2_OC3FE | TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M_1 ;
	TIM1->DIER |= TIM_DIER_CC1DE | TIM_DIER_CC3DE;
	TIM1->BDTR = TIM_BDTR_MOE;
	TIM1->PSC = timer1Prescaler;
	TIM1->ARR = 1000;
	TIM1->CCR1 = 0;
	TIM1->CCR3 = 0;
	TIM1->CR1 = TIM_CR1_ARPE; //| TIM_CR1_CMS;
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
	
	timerEvents.timerInitType = TIMER_INIT_AS_DMA;

  return 1;
}

uint8_t GenInitTimer1ForPWM(void)
{
	GenSignalEmulStop();
	GenSetPorts();
	
	//вкл Timer1
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	//инициализация Timer1
	TIM1->CR1 &= ~TIM_CR1_CEN;	
  TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC3E;
	TIM1->CCMR1 = TIM_CCMR1_OC1FE | TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
  TIM1->CCMR2 = TIM_CCMR2_OC3FE | TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M;
	TIM1->BDTR = TIM_BDTR_MOE;
	TIM1->PSC = timer1Prescaler;
	TIM1->ARR = 1000;
	TIM1->CCR1 = 0;
	TIM1->CCR3 = 1000;
	TIM1->CR1 = TIM_CR1_ARPE | TIM_CR1_CMS;
	TIM1->CR1 |= TIM_CR1_CEN;

  timerEvents.timerInitType = TIMER_INIT_AS_PWM;	
	
  return 1;
}

uint8_t GenInitSignalOnTimer1(uint8_t _signal_type)
{
	//проверка выбора типа сигнала
	if ( ( _signal_type > signalIndexMax ) || ( _signal_type < signalIndexMin ) ) return 0;
	
	if ( _signal_type == signalSquare )
	{
		//инициализация как шим меандр
		return GenInitTimer1ForPWM();
	} else
	{
		//инициализация как эмулятор сигналов
		return GenInitTimer1ForSignalEmul();
	};
	
	return 0;
}

void GenTimer1SetPrescaler(void) 
{
	if ( TIM1->PSC != timer1Prescaler )
	{
		TIM1->PSC = timer1Prescaler;
	}
}

uint8_t GenTimer1ForPWMUpdate(uint16_t _tim_prescaler, uint16_t _tim_arr, uint16_t _tim_ccr,
                              uint32_t _transistorsMinTimeNS, uint32_t _transistorsDeadTimeNS)
{
	uint16_t _ccr, _ccr3, transistorsMinStep, transistorsMaxStep;
	
	//установка предделителя
	timer1Prescaler = _tim_prescaler;
	GenTimer1SetPrescaler();
	//установка регистра сравнения
	_ccr = _tim_ccr;
	//min time - NS -> в циклы
	transistorsMinStep = (double)_transistorsMinTimeNS * cpuFreq / ( (_tim_prescaler + 1) * 2 * 1000000000 );	
	//dead time - NS -> в циклы
	transistorsMaxStep = (double)_transistorsDeadTimeNS * cpuFreq / ( (_tim_prescaler + 1) * 2 * 1000000000 );
	//control compare reg
	if ( _ccr < transistorsMinStep ) _ccr = transistorsMinStep;
	if ( ( ( ( _tim_arr / 2 ) - _ccr) < transistorsMaxStep ) || ( _ccr > ( _tim_arr / 2 ) ) )
	{
		_ccr = ( _tim_arr / 2 ) - transistorsMaxStep;
	}
	_ccr3 = _tim_arr - _ccr;
	//обновление регистра управления
	TIM1->CR1 |= TIM_CR1_ARPE | TIM_CR1_CMS;
	//сброс и установка
	TIM1->ARR = _tim_arr;
	TIM1->CCR1 = _ccr;
	TIM1->CCR3 = _ccr3;
	
	return 1;
}

void calculateSinArray(uint16_t *_sin_array, uint16_t _arr_size, uint16_t _acc_sin,
	                     double _power_k, double _center_k, uint32_t _transistorsMinTimeNS, uint32_t _transistorsDeadTimeNS)
{
	uint16_t i = 0, j = 0;
	double sin_element = 0;
	double _center_mult = 0;
	double vector = 0;
	//min time - from NS - to cicles
	uint16_t transistorsMinStep = (double)_transistorsMinTimeNS * cpuFreq / 1000000000;	
	//dead time - from NS - to cicles
	uint16_t transistorsMaxStep = (double)_acc_sin - (double)_transistorsDeadTimeNS * cpuFreq / 1000000000;	
	//k of pulse center - from +/- XXXX.XX - to 0.XXXXXX ... X.XXXXX
	_center_mult = ( _center_k >= 0 ) ? ( (_center_k + 100) / 100 ) : ( 100 / fabs(_center_k - 100) );
	// 1/2 up
	while ( ( sin_element >= vector ) && ( i < _arr_size ) )
	{
		vector = sin_element;
		sin_element = (double)sin( i * pi * _center_mult / _arr_size ) * _acc_sin * _power_k;		
		_sin_array[i] = sin_element;
		//контроль на минимальный DutyCicle
		if ( _sin_array[i] < transistorsMinStep ) _sin_array[i] = 0;
		//контроль для DeadTime отдельно, для прозрачности
		if ( _sin_array[i] > transistorsMaxStep ) _sin_array[i] = transistorsMaxStep;	
    i++;		
	}
	// 1/2 down
	if ( i < _arr_size )
	{
		if ( i > 0 ) i--;
		j = i;
		vector = pi / ( 2 * ( _arr_size - j ) );
		while ( i < _arr_size )
		{
			sin_element = (double)sin( pi / 2 + (i - j + 1) * vector ) * _acc_sin * _power_k;
		  _sin_array[i] = sin_element;
		  //контроль на минимальный DutyCicle
		  if ( _sin_array[i] < transistorsMinStep ) _sin_array[i] = 0;
		  //контроль для DeadTime отдельно, для прозрачности
		  if ( _sin_array[i] > transistorsMaxStep ) _sin_array[i] = transistorsMaxStep;	
      i++;
		}
	}
}

void calculateTriangleArray(uint16_t *_tri_array, uint16_t _arr_size, uint16_t _acc_tri,
		                        double _power_k, double _center_k, uint32_t _transistorsMinTimeNS, uint32_t _transistorsDeadTimeNS)
{
	uint16_t i = 0;
	double _center_mult = 0;
	double _a, _b = 0;
	//min time - from NS - to cicles
	uint16_t transistorsMinStep = (double)_transistorsMinTimeNS * cpuFreq / 1000000000;	
	//dead time - from NS - to cicles
	uint16_t transistorsMaxStep = (double)_acc_tri - (double)_transistorsDeadTimeNS * cpuFreq / 1000000000;	
	//k of pulse center - from +/- XXXX.XX - to 0.0001 ... 1.9999
	_center_mult = ( _center_k >= 0 ) ? ( _center_k / 100 + 1 ) : ( 1 - (fabs(_center_k) / 100) );
	if ( _center_mult < 0 ) _center_mult = 0.0001;
	if ( _center_mult > 2 ) _center_mult = 1.9999;
	//1/2 up
	//coeff
	_a = _arr_size * _center_mult / 2;
	if ( ( _a == 0 ) || ( (_arr_size - _a - 1) == 0 ) ) return;
	_b = (double)_acc_tri / _a;	
	for (i = 0; i < _a + 1; i++){
		_tri_array[i] = (double)_b * i * _power_k;
		//контроль на минимальный DutyCicle
		if ( _tri_array[i] < transistorsMinStep ) _tri_array[i] = 0;
		//контроль для DeadTime отдельно, для прозрачности
		if ( _tri_array[i] > transistorsMaxStep ) _tri_array[i] = transistorsMaxStep;
	};
	//1/2 down
	//coeff
	_b = (double)_acc_tri / (_arr_size - _a - 1);
	for (i = _a + 1; i < _arr_size; i++){
		_tri_array[i] = (double)( _acc_tri - _b * (i - _a) ) * _power_k;
		//контроль на минимальный DutyCicle
		if ( _tri_array[i] < transistorsMinStep ) _tri_array[i] = 0;
		//контроль для DeadTime отдельно, для прозрачности
		if ( _tri_array[i] > transistorsMaxStep ) _tri_array[i] = transistorsMaxStep;
	};
}

void calculateDataArray(uint16_t *_signal_array, uint16_t _arr_size, uint16_t _acc_signal,
	                      double _power_k, double _center_k, uint32_t _transistorsMinTimeNS,
                        uint32_t _transistorsDeadTimeNS, uint8_t _signal_type)
{
  switch ( _signal_type )
	{
		case 1: //sinus
			calculateSinArray(_signal_array, _arr_size, _acc_signal, _power_k, _center_k, _transistorsMinTimeNS, _transistorsDeadTimeNS);
		break;
		
		case 2: //triangle
			calculateTriangleArray(_signal_array, _arr_size, _acc_signal, _power_k, _center_k, _transistorsMinTimeNS, _transistorsDeadTimeNS);
		break;
	}		
};

uint8_t GenUpdateSignal(const uint16_t _tim_prescaler, const uint16_t _tim_arr, const uint16_t _tim_steps_ccr,
	                      const double _power_k, const double _center_k, const uint32_t _transistorsMinTimeNS,
												const uint32_t _transistorsDeadTimeNS, const uint8_t _signal_type)
{
	//проверка выбора типа сигнала
	if ( ( _signal_type > signalIndexMax ) || ( _signal_type < signalIndexMin ) ) return 0;
	
	//обработка при сигнале = 3 (меандр)
	if ( _signal_type == signalSquare )
	{
		//проверка подходящего режима таймера и включение
		if ( timerEvents.timerInitType != TIMER_INIT_AS_PWM )
		{
			if ( !GenInitSignalOnTimer1(signalSquare) ) return 0; 
		};
		return GenTimer1ForPWMUpdate(_tim_prescaler, _tim_arr, _tim_steps_ccr, _transistorsMinTimeNS, _transistorsDeadTimeNS);
	};
	
	//проверка размера сигнала относительно размера буфера
	if ( _tim_steps_ccr > maxSignSteps ) return 0;	
	//проверка коэффиицента мощности
	if ( ( _power_k > MAX_POWER_K ) || ( _power_k < MIN_POWER_K ) ) return 0;
	
	//проверка подходящего режима таймера и включение
	if ( timerEvents.timerInitType != TIMER_INIT_AS_DMA )
	{
		if ( !GenInitSignalOnTimer1(_signal_type) ) return 0; 
	};
	
	//принудительно убираем флаг
	timerEvents.mayChangeArray = 0;
	//устанавливаем предделитель
	timer1Prescaler = _tim_prescaler;
	//выбор массива и заполнение
	if ( timerEvents.currentArray == TIMER_CURRENT_A ) //если текущий буфер - А
	{
		accuracySignalB = _tim_arr;
		pwmSignalStepsB = _tim_steps_ccr;
		calculateDataArray(pwmSignalArrayB, pwmSignalStepsB, accuracySignalB, _power_k/100, _center_k,
		                   _transistorsMinTimeNS, _transistorsDeadTimeNS, _signal_type);
	} else
	{ //теущий буфер - B
		accuracySignalA = _tim_arr;
		pwmSignalStepsA = _tim_steps_ccr;
		calculateDataArray(pwmSignalArrayA, pwmSignalStepsA, accuracySignalA, _power_k/100, _center_k,
		                   _transistorsMinTimeNS, _transistorsDeadTimeNS, _signal_type);
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
	if ( timerEvents.currentArray == TIMER_CURRENT_A ) //текущий буфер - А
	{
		DMA1_Channel2->CMAR = (uint32_t) &pwmSignalArrayB[0];
		DMA1_Channel6->CMAR = (uint32_t) &pwmSignalArrayB[0];
	  DMA1_Channel2->CNDTR = pwmSignalStepsB;
	  DMA1_Channel6->CNDTR = pwmSignalStepsB;
	  //check prescaler
	  GenTimer1SetPrescaler();
		//set registers
		TIM1->CR1 |= TIM_CR1_ARPE;
	  TIM1->ARR = accuracySignalB;
	  TIM1->CCR1 = 0;
	  TIM1->CCR3 = 0;
		//переключение флага на B
		timerEvents.currentArray = TIMER_CURRENT_B;
    timerEvents.signalEnabled = 1;		
	} else 
	{
		//текущий буфер - B
		DMA1_Channel2->CMAR = (uint32_t) &pwmSignalArrayA[0];
	  DMA1_Channel6->CMAR = (uint32_t) &pwmSignalArrayA[0];
	  DMA1_Channel2->CNDTR = pwmSignalStepsA;
	  DMA1_Channel6->CNDTR = pwmSignalStepsA;
	  //check prescaler
	  GenTimer1SetPrescaler();
		//set registers
		TIM1->CR1 |= TIM_CR1_ARPE;
	  TIM1->ARR = accuracySignalA;
	  TIM1->CCR1 = 0;
	  TIM1->CCR3 = 0;
		//переключение флага на A
		timerEvents.currentArray = TIMER_CURRENT_A;
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

uint8_t GenCheckSignalConfig(const int32_t _tim_prescaler, const int32_t _tim_arr, const int32_t _tim_steps_ccr,
	                           const double _power_k, const double _center_k, const uint32_t _transistorsMinTimeNS,
												     const uint32_t _transistorsDeadTimeNS, const uint8_t _signal_type)
{
	uint16_t transistorsMinStep, transistorsMaxStep;
	
	if ( ( _tim_prescaler < 0 ) || ( _tim_prescaler >= USHRT_MAX ) ) return 0;
	if ( ( _tim_arr < 0 ) || ( _tim_arr > USHRT_MAX ) ) return 0;
	if ( ( _tim_steps_ccr < 0 ) || ( _tim_steps_ccr > USHRT_MAX ) ) return 0;
	if ( ( _transistorsMinTimeNS < MIN_PULSE_TIME ) || ( _transistorsMinTimeNS > MAX_PULSE_TIME ) ) return 0;
	if ( ( _transistorsDeadTimeNS < MIN_DEAD_TIME ) || ( _transistorsDeadTimeNS > MAX_DEAD_TIME ) ) return 0;
	
	//min time - NS -> в циклы
	transistorsMinStep = (double)_transistorsMinTimeNS * cpuFreq / ( (_tim_prescaler + 1) * 2 * 1000000000 );	
	//dead time - NS -> в циклы
	transistorsMaxStep = (double)_transistorsDeadTimeNS * cpuFreq / ( (_tim_prescaler + 1) * 2 * 1000000000 );
  if ( ( _signal_type == signalSquare ) && ( _tim_steps_ccr > ( ( _tim_arr / 2 ) - transistorsMaxStep ) ) ) return 0;
	if ( ( _signal_type == signalSquare ) && ( _tim_steps_ccr < transistorsMinStep ) ) return 0;
	if ( _tim_arr < 2 ) return 0;
	if ( ( ( _tim_steps_ccr < 2 ) || ( _tim_steps_ccr > maxSignSteps ) ) &&
		   ( ( _signal_type == signalSinus ) || ( _signal_type == signalTriangle ) ) ) return 0;
	if ( ( _power_k < MIN_POWER_K ) || ( _power_k > MAX_POWER_K ) ) return 0;
	if ( ( _center_k < MIN_CENTER_K ) || ( _center_k > MAX_CENTER_K ) ) return 0;
	if ( ( _signal_type < signalIndexMin ) || ( _signal_type > signalIndexMax ) ) return 0;
		
	return 1;
}

uint16_t GenGetPrescalerValue(const double _freq_pwm, const uint8_t _signal_type)
{
	uint16_t _prescaler_value = 0;
	if ( _freq_pwm == 0.0 ) return USHRT_MAX;
	if ( _signal_type == signalSquare )
	{
		_prescaler_value = (uint16_t)( (double)cpuFreq / ( (double)_freq_pwm * 131070 ) );		
	}
	if ( ( _signal_type == signalSinus ) || ( _signal_type == signalTriangle ) )
	{
		_prescaler_value = (uint16_t)( (double)cpuFreq / ( (double)_freq_pwm * 65535 ) );
	}
	return _prescaler_value;
}

uint16_t GenGetARRValueFromFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const uint8_t _signal_type)
{
	uint16_t _arr_value = 0;
	if ( _freq_pwm == 0.0 ) return 0;
	if ( _tim_prescaler == USHRT_MAX ) return 0;
	if ( _signal_type == signalSquare )
	{
		_arr_value = (uint16_t)( (double)cpuFreq / ( (double)(_tim_prescaler + 1) * 2 * _freq_pwm ) );		
	}
	if ( ( _signal_type == signalSinus ) || ( _signal_type == signalTriangle ) )
	{
		_arr_value = (uint16_t)( (double)cpuFreq / ( (double)(_tim_prescaler + 1) * _freq_pwm ) );
	}	
	if ( _arr_value < 2 ) _arr_value = 0;
	return _arr_value;
}

uint16_t GenGetStepsCCRValueFromFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal,
	                                   const uint16_t _tim_arr, const double _power_k, const uint32_t _transistorsMinTimeNS,
																		 const uint32_t _transistorsDeadTimeNS, const uint8_t _signal_type)
{
	uint16_t transistorsMinStep, transistorsMaxStep;
		
	uint16_t _steps_value = 0;
	if ( !_tim_arr ) return 0;
	if ( _freq_pwm == 0.0 ) return 0;
	if ( _freq_signal == 0.0 ) return 0;
	if ( _signal_type == signalSquare )
	{
		_steps_value = (uint16_t)( (double)_tim_arr * _power_k / 2 );
		//min time - NS -> в циклы
	  transistorsMinStep = (double)_transistorsMinTimeNS * cpuFreq / ( (_tim_prescaler + 1) * 2 * 1000000000 );	
	  //dead time - NS -> в циклы
	  transistorsMaxStep = (double)_transistorsDeadTimeNS * cpuFreq / ( (_tim_prescaler + 1) * 2 * 1000000000 );
	  if ( _steps_value < transistorsMinStep ) _steps_value = transistorsMinStep;
	  if ( ( ( ( _tim_arr / 2 ) - _steps_value) < transistorsMaxStep ) || ( _steps_value > ( _tim_arr / 2 ) ) )
	  {
		  _steps_value = ( _tim_arr / 2 ) - transistorsMaxStep;
	  }	
	}
	if ( ( _signal_type == signalSinus ) || ( _signal_type == signalTriangle ) )
	{
		_steps_value = (uint16_t)( _freq_pwm / ( 2.0 * _freq_signal ) );
	  if ( _steps_value < 2 ) return 0;
	  if ( _steps_value > maxSignSteps ) return 0;
	}
	return _steps_value;
}

double GenGetPWMFreqValueFromTimer(const uint16_t _tim_prescaler, const uint16_t _tim_arr, const uint8_t _signal_type)
{
  if ( !GenCheckSignalConfig(_tim_prescaler, _tim_arr, 0, MIN_POWER_K,
		                         MIN_CENTER_K, MIN_PULSE_TIME, MIN_DEAD_TIME, _signal_type) ) return 0;
	
	if ( _signal_type == signalSquare )
	{
		return ( (double)cpuFreq / ( (double)(_tim_prescaler + 1) * 2 * _tim_arr ) );
	}
	if ( ( _signal_type == signalSinus ) || ( _signal_type == signalTriangle ) )
	{
		return ( (double)cpuFreq / ( (double)(_tim_prescaler + 1) * _tim_arr ) );
	}
	return -1;
}

double GenGetSignalFreqValueFromTimer(const uint16_t _tim_prescaler, const uint16_t _tim_arr, const uint16_t _tim_steps_ccr, const uint8_t _signal_type)
{
  if ( !GenCheckSignalConfig(_tim_prescaler, _tim_arr, _tim_steps_ccr, MIN_POWER_K,
		                         MIN_CENTER_K, MIN_PULSE_TIME, MIN_DEAD_TIME, _signal_type) ) return 0;
	
	if ( _signal_type == signalSquare )
	{
		return ( (double)cpuFreq / ( (double)(_tim_prescaler + 1) * 2 * _tim_arr ) );
	}
	if ( ( _signal_type == signalSinus ) || ( _signal_type == signalTriangle ) )
	{
		return ( (double)cpuFreq / ( (double)(_tim_prescaler + 1) * _tim_arr * 2.0 * _tim_steps_ccr ) );
	}
	return -1;
}

uint8_t GenCheckFrequencyAvailable(const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type)
{
	uint32_t a = 0, b = 0, c = 0;
	a = GenGetPrescalerValue(_freq_pwm, _signal_type);
	b = GenGetARRValueFromFreq(a, _freq_pwm, _signal_type);
	c = GenGetStepsCCRValueFromFreq(a, _freq_pwm, _freq_signal, b, MIN_POWER_K, MIN_PULSE_TIME, MIN_DEAD_TIME, _signal_type);
	c = b && ( ( ( _signal_type == signalSquare ) && ( c <= ( b / 2 ) ) ) || ( ( _signal_type != signalSquare ) && c ) );
	return (uint8_t)( ( a != USHRT_MAX ) && c );
}

double GetRealAvailablePWMFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type)
{
	double _new_freq = 0;
  if ( !GenCheckFrequencyAvailable(_freq_pwm, _freq_signal, _signal_type) ) return 0;
	
	_new_freq = GenGetARRValueFromFreq(_tim_prescaler, _freq_pwm, _signal_type);
	_new_freq = GenGetPWMFreqValueFromTimer(_tim_prescaler, _new_freq, _signal_type);
	if ( _new_freq <= 0.0 )
	{
		return _freq_pwm;
	} else
	{
		return _new_freq;
	}
}
double GetNextAvailablePWMFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type)
{
	double _new_freq = 0;
  if ( !GenCheckFrequencyAvailable(_freq_pwm, _freq_signal, _signal_type) ) return 0;
	
	_new_freq = GenGetARRValueFromFreq(_tim_prescaler, _freq_pwm, _signal_type);
	_new_freq--;
	_new_freq = GenGetPWMFreqValueFromTimer(_tim_prescaler, _new_freq, _signal_type);
	if ( _new_freq <= 0.0 )
	{
		return _freq_pwm;
	} else
	{
		return _new_freq;
	}
}

double GetPrevAvailablePWMFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type)
{
	double _new_freq = 0;
  if ( !GenCheckFrequencyAvailable(_freq_pwm, _freq_signal, _signal_type) ) return 0;
	
	_new_freq = GenGetARRValueFromFreq(_tim_prescaler, _freq_pwm, _signal_type);
	_new_freq++;
	if ( _new_freq > USHRT_MAX ) return _freq_pwm;
	_new_freq = GenGetPWMFreqValueFromTimer(_tim_prescaler, _new_freq, _signal_type);
	if ( _new_freq <= 0.0 )
	{
		return _freq_pwm;
	} else
	{
		return _new_freq;
	}
}

double GetRealAvailableSignalFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type)
{
	double _new_freq = 0;
	uint16_t _arr = 0;
  if ( !GenCheckFrequencyAvailable(_freq_pwm, _freq_signal, _signal_type) ) return 0;
	if ( _signal_type == signalSquare ) return 0;
	
	_arr = GenGetARRValueFromFreq(_tim_prescaler, _freq_pwm, _signal_type);
	_new_freq = GenGetStepsCCRValueFromFreq(_tim_prescaler, _freq_pwm, _freq_signal, _arr, MIN_POWER_K, MIN_PULSE_TIME, MIN_DEAD_TIME, _signal_type);
	_new_freq = GenGetSignalFreqValueFromTimer(_tim_prescaler, _arr, _new_freq, _signal_type); 
	return _new_freq;
}

double GetNextAvailableSignalFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type)
{
	double _new_freq = 0;
	uint16_t _arr = 0;
  if ( !GenCheckFrequencyAvailable(_freq_pwm, _freq_signal, _signal_type) ) return 0;
	if ( _signal_type == signalSquare ) return 0;
	
	_arr = GenGetARRValueFromFreq(_tim_prescaler, _freq_pwm, _signal_type);
	_new_freq = GenGetStepsCCRValueFromFreq(_tim_prescaler, _freq_pwm, _freq_signal, _arr, MIN_POWER_K, MIN_PULSE_TIME, MIN_DEAD_TIME, _signal_type);
	_new_freq--;
	_new_freq = GenGetSignalFreqValueFromTimer(_tim_prescaler, _arr, _new_freq, _signal_type);
  if ( _new_freq <= 0.0 )
	{
		return _freq_signal;
	} else
  {
		return _new_freq;
	}
}

double GetPrevAvailableSignalFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type)
{
	double _new_freq = 0;
	uint16_t _arr = 0;
  if ( !GenCheckFrequencyAvailable(_freq_pwm, _freq_signal, _signal_type) ) return 0;
	if ( _signal_type == signalSquare ) return 0;
	
	_arr = GenGetARRValueFromFreq(_tim_prescaler, _freq_pwm, _signal_type);
	_new_freq = GenGetStepsCCRValueFromFreq(_tim_prescaler, _freq_pwm, _freq_signal, _arr, MIN_POWER_K, MIN_PULSE_TIME, MIN_DEAD_TIME, _signal_type);
	_new_freq++;
	_new_freq = GenGetSignalFreqValueFromTimer(_tim_prescaler, _arr, _new_freq, _signal_type);
  if ( _new_freq <= 0.0 )
	{
		return _freq_signal;
	} else
  {
		return _new_freq;
	}
}
