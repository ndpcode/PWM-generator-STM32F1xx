
#include "settings.h"
#include "gen_timers.h"
#include <math.h>
#include <string.h>

uint16_t accuracySignalA, accuracySignalB; //�������� ���, ������� �������� ���������� �� ������������ � ������ ���
uint16_t pwmSignalStepsA, pwmSignalStepsB; //����� �������� ��� � ����� ����������� �������
uint16_t pwmSignalArrayA[maxSinSteps];  //������ � ��� ������� �������
uint16_t pwmSignalArrayB[maxSinSteps];  //������ B ��� ������� �������    
int8_t signalH, signalL;

struct timerEventsBase
{
	unsigned currentArray : 1; //������� ������������ ������ � ��������� ������� ( A (0) ��� B (1) )
	unsigned mayChangeArray : 1; //���� ����������� ����� ������� � ���������
	unsigned signalEnabled : 1;
} timerEvents;

uint8_t GenInitTimers(void)
{
	memset( &timerEvents, 0, sizeof(timerEvents) );
  memset( pwmSignalArrayA, 0, sizeof(uint16_t) * maxSinSteps );	
	memset( pwmSignalArrayB, 0, sizeof(uint16_t) * maxSinSteps );
	signalH = 1;
	signalL = 0;
	
	//��� Timer1
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	//��������� Timer1
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
	
	//��������� ��������
	pwmSignalStepsA = 10;
	//��� DMA, DMA->Timer1
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
		//�������� ��� DeadTime ��������, ��� ������������
		if ( _sin_array[i] > transistorsMaxStep ) _sin_array[i] = transistorsMaxStep;
	};
	//������� �������� ������������� � 0
	//_sin_array[0] = 0;
	//_sin_array[_arr_size-1] = 0;
}

void createSinArray(uint32_t _freq_pwm, uint32_t _freq_sin, double _power_k)
{	
	if ( !timerEvents.currentArray ) //���� ������� ����� - �
	{
		accuracySignalB = cpuFreq / ( _freq_pwm );
		pwmSignalStepsB = _freq_pwm / ( 2*_freq_sin );
		calculateSinArray(pwmSignalArrayB, pwmSignalStepsB, accuracySignalB, _power_k);
	} else
	{ //������ ����� - B
		accuracySignalA = cpuFreq / ( _freq_pwm );
		pwmSignalStepsA = _freq_pwm / ( 2*_freq_sin );
		calculateSinArray(pwmSignalArrayA, pwmSignalStepsA, accuracySignalA, _power_k);
	};	
};

//________________startSignal____________________
uint8_t updateSignal(uint32_t _freq_pwm, uint32_t _freq_signal, double _power_k, uint8_t _signal_type)
{
	//������������� ������� ����
	timerEvents.mayChangeArray = 0;
	//����� ������� � ���������� �������
	switch ( _signal_type )
	{
		case 0: //sinus
			createSinArray(_freq_pwm, _freq_signal, _power_k);			
		break;
	};	
	//���������� ������ � �������� ������������ �������
	//�� ���������� DMA ����� ������� ������ �������
	timerEvents.mayChangeArray = 1;
	
	return 1;
};

void DMA_Interrupt_Change_Signal(void)
{
	//������������� ���� DMA
	DMA1_Channel2->CCR &= 0xFFFFFFFE;
	DMA1_Channel6->CCR &= 0xFFFFFFFE;
	//����� �����
	if ( !timerEvents.currentArray ) //������� ����� - �
	{
		DMA1_Channel2->CMAR = (uint32_t) &pwmSignalArrayB[0];
		DMA1_Channel6->CMAR = (uint32_t) &pwmSignalArrayB[0];
	  DMA1_Channel2->CNDTR = pwmSignalStepsB;
	  DMA1_Channel6->CNDTR = pwmSignalStepsB;
	  TIM1->ARR = accuracySignalB;
	  TIM1->CCR1 = 0;
	  TIM1->CCR3 = 0;
		//������������ ����� �� B
		timerEvents.currentArray = 1;
    timerEvents.signalEnabled = 1;		
	} else
	{
		//������� ����� - B
		DMA1_Channel2->CMAR = (uint32_t) &pwmSignalArrayA[0];
	  DMA1_Channel6->CMAR = (uint32_t) &pwmSignalArrayA[0];
	  DMA1_Channel2->CNDTR = pwmSignalStepsA;
	  DMA1_Channel6->CNDTR = pwmSignalStepsA;
	  TIM1->ARR = accuracySignalA;
	  TIM1->CCR1 = 0;
	  TIM1->CCR3 = 0;
		//������������ ����� �� A
		timerEvents.currentArray = 0;
		timerEvents.signalEnabled = 1;
	};
	// ����
	timerEvents.mayChangeArray = 0;
}

void DMA_Interrupt_Change_Period(void)
{
	if(signalH == 1) {
		//���� 2
		CH2_DOWN;
		//���� 1
		TIM1->CCMR1 &= ~TIM_CCMR1_OC1M_2;
		//������ ������
		if ( timerEvents.mayChangeArray ) DMA_Interrupt_Change_Signal();
		//��� 3
		TIM1->CCMR2 |=  TIM_CCMR2_OC3M_2;
		//��� 4
		if ( timerEvents.signalEnabled ) CH4_UP;
		DMA1_Channel2->CCR &= 0xFFFFFFFE;
		DMA1_Channel6->CCR |= DMA_CCR6_EN;
	}
	else  {
		//���� 4
		CH4_DOWN;
		//���� 3
		TIM1->CCMR2 &= ~TIM_CCMR2_OC3M_2;
		//������ ������
		if ( timerEvents.mayChangeArray ) DMA_Interrupt_Change_Signal();
		//��� 1
		TIM1->CCMR1 |=  TIM_CCMR1_OC1M_2;		
		//��� 2
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
