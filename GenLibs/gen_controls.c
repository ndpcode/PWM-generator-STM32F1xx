
#include "settings.h"
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
	unsigned button_busy_flag : 1;
	uint32_t event_code;
	void *next_button;
} ControlsBaseStruct;

ControlsBaseStruct *ButtonsData;
ControlsBaseStruct *ValcoderData;

ControlsBaseStruct* getLastItem(ControlsBaseStruct *_data)
{
	ControlsBaseStruct *last_element = _data;
	if ( !_data ) return 0;
	if ( !_data->next_button ) return _data;
	while ( last_element->next_button )
	{
		last_element = (ControlsBaseStruct*)last_element->next_button;
	};
	return last_element;
}

uint8_t ControlsDataClear(void)
{
	ControlsBaseStruct **last_element;
	if ( ButtonsData )
	{
		*last_element = getLastItem(ButtonsData);
		while ( *last_element )
		{
			free(*last_element);
			*last_element = getLastItem(ButtonsData);
		};
	};
	if ( ValcoderData )
	{
		*last_element = getLastItem(ValcoderData);
		while ( *last_element )
		{
			free(*last_element);
			*last_element = getLastItem(ValcoderData);
		};
	};		
	return 1;
};

uint8_t ControlsRegNewButton(char _portLetter, uint8_t _pinNumber, void (*_click_func)(void), uint32_t _event_code)
{
	ControlsBaseStruct *new_button = 0;
	ControlsBaseStruct *last_button = getLastItem(ButtonsData);
	//проверка буквы порта
	if ( !strchr("ABCDEF", _portLetter) ) return 0;
	//проверка номера пина
	if ( _pinNumber > 15 ) return 0;
	
	//создаем новый элемент
	new_button = (ControlsBaseStruct*)malloc( sizeof(ControlsBaseStruct) );
	//проверка и выход если память не выделена
	if ( !new_button  ) return 0;
	if ( !ButtonsData )
	{
		ButtonsData = new_button; 
	} else if ( last_button )
	{
		last_button->next_button = new_button;
	};
	//проверка
	if ( ( !ButtonsData ) && ( !last_button ) )
	{
		//удаляем, что-то пошло не так...
		free(new_button);
		return 0;
	};
	//заполняем данные элемента
	switch ( _portLetter )
	{
		case 'A':
			new_button->port_idr_addr = &GPIOA->IDR;			
		break;
		case 'B':
			new_button->port_idr_addr = &GPIOB->IDR;			
		break;
		case 'C':
			new_button->port_idr_addr = &GPIOC->IDR;			
		break;
		case 'D':
			new_button->port_idr_addr = &GPIOD->IDR;			
		break;
	  case 'E':
			new_button->port_idr_addr = &GPIOE->IDR;			
		break;
		case 'F':
			new_button->port_idr_addr = &GPIOF->IDR;			
		break;
	};
	new_button->pin_addr = 1 << _pinNumber;
	new_button->_button_click_func = _click_func;
  new_button->event_code = _event_code;
  return 1;	
};

uint8_t ControlsRegNewValcoder(char _portLetterCCW, uint8_t _pinNumberCCW,
	                             char _portLetterCW, uint8_t _pinNumberCW,
															 void (*_ccw_func)(void), uint32_t _event_ccw_code,
															 void (*_cw_func)(void), uint32_t _event_cw_code)
{
};

uint32_t ControlsUpdateEvents(UpdateEventsParams _params)
{
}

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
