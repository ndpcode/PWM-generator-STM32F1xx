
#include "stm32f10x.h"
#include "gen_controls.h"
#include <stdlib.h>
#include <string.h>

typedef struct 
{
	volatile uint32_t *port_idr_addr;
	uint32_t pin_addr;
	void ( *_button_click_func )(void);
	void ( *_button_double_click_func )(void);
	void ( *_button_hold_func )(void);
	unsigned button_pressed_flag : 1;
	unsigned button_busy_flag : 1;
	unsigned button_double_click_flag : 1;
	unsigned button_hold_flag : 1;
} ButtonsBase;

ButtonsBase *ButtonsData;
uint8_t ButtonsLength = 0;

void ButtonsDataClear(void)
{
	free(ButtonsData);
	ButtonsLength = 0;
};

void ButtonsAddNewButton(char _portLetter, uint8_t _pinNumber, void (*_click_func)(void))
{
	//выделяем память
	ButtonsLength++;
	ButtonsData = (ButtonsBase*)realloc(ButtonsData, ButtonsLength * sizeof(ButtonsBase) );
	memset( &ButtonsData[ButtonsLength-1], 0, sizeof(ButtonsBase) );
	//заполняем данные элемента
	switch ( _portLetter )
	{
		case 'A':
			ButtonsData[ButtonsLength-1].port_idr_addr = &GPIOA->IDR;			
		break;
		case 'B':
			ButtonsData[ButtonsLength-1].port_idr_addr = &GPIOB->IDR;			
		break;
		case 'C':
			ButtonsData[ButtonsLength-1].port_idr_addr = &GPIOC->IDR;			
		break;
		case 'D':
			ButtonsData[ButtonsLength-1].port_idr_addr = &GPIOD->IDR;			
		break;
	  case 'E':
			ButtonsData[ButtonsLength-1].port_idr_addr = &GPIOE->IDR;			
		break;
		case 'F':
			ButtonsData[ButtonsLength-1].port_idr_addr = &GPIOF->IDR;			
		break;
	};
	ButtonsData[ButtonsLength-1].pin_addr = 1 << _pinNumber;
	ButtonsData[ButtonsLength-1]._button_click_func = _click_func;  	
};

void ButtonsCheck(void)
{
	int i = 0;
	//опрос кнопок
	for (i = 0; i < ButtonsLength; i++)
	{
		//проверяем низкий уровень на пин
		if ( ( *ButtonsData[i].port_idr_addr & ButtonsData[i].pin_addr ) == 0 )
		{
			if ( !ButtonsData[i].button_busy_flag )
			{
				//если кнопка нажата впервые
				//флаг
				ButtonsData[i].button_busy_flag = 1;
				//событие по кнопке
				ButtonsData[i]._button_click_func();
			} else
			{
				//кнопка срабатывает повторно/удерживается
				ButtonsData[i].button_busy_flag = 1;
			};
		} else
		{
			//если кнопка отпущена, сбрасываем флаг
			if ( ButtonsData[i].button_busy_flag )
			{
				ButtonsData[i].button_busy_flag = 0;
			};				
		};
	};
};
