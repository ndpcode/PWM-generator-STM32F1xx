
#include "settings.h"
#include "gen_controls.h"
#include <stdlib.h>
#include <string.h>

typedef struct 
{
	volatile uint32_t *port_idr_addr;
	uint32_t pin_addr;
	void ( *_control_click_func )(void);
	unsigned control_busy_flag : 1;
	uint32_t event_code;
	void *next_control_element;
} ControlsBaseStruct;

ControlsBaseStruct *ButtonsData;
ControlsBaseStruct *ValcoderData;

ControlsBaseStruct* getLastItem(ControlsBaseStruct *_data)
{
	ControlsBaseStruct *last_element = _data;
	if ( !_data ) return 0;
	if ( !_data->next_control_element ) return _data;
	while ( last_element->next_control_element )
	{
		last_element = (ControlsBaseStruct*)last_element->next_control_element;
	};
	return last_element;
}

volatile uint32_t* getPortFromLetter(char _portLetter)
{	
	//без проверок буквы порта
	//возвращаем адрес на порт
	switch ( _portLetter )
	{
		case 'A': return &GPIOA->IDR;			
		case 'B': return &GPIOB->IDR;			
		case 'C': return &GPIOC->IDR;			
		case 'D': return &GPIOD->IDR;			
	  case 'E': return &GPIOE->IDR;			
		case 'F': return &GPIOF->IDR;
	};
	return 0;
}

ControlsBaseStruct* pushNewElement(ControlsBaseStruct *_data)
{
	//добавляем новый элемент
	ControlsBaseStruct *new_control = 0;
	ControlsBaseStruct *last_control = getLastItem(_data);
	//создаем новый элемент
	new_control = (ControlsBaseStruct*)malloc( sizeof(ControlsBaseStruct) );
	//проверка и выход если память не выделена
	if ( !new_control  ) return 0;
	if ( !_data )
	{
		_data = new_control; 
	} else if ( last_control )
	{
		last_control->next_control_element = new_control;
	};
	//проверка
	if ( ( !_data ) && ( !last_control ) )
	{
		//удаляем, что-то пошло не так...
		free(new_control);
		return 0;
	};
	return new_control;
}

ControlsBaseStruct* popLastElement(ControlsBaseStruct *_data)
{
	//удаляем последний элемент и возвращаем указатель на предпоследний
	ControlsBaseStruct *last_control = 0;
	ControlsBaseStruct *penult_control = 0;
	if ( !_data ) return 0;
	last_control = _data;
	while ( last_control->next_control_element )
	{
		penult_control = last_control;
		last_control = (ControlsBaseStruct*)last_control->next_control_element;
	};
	free(last_control);
	penult_control->next_control_element = 0;
	return penult_control;
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
	//проверка буквы порта
	if ( !strchr("ABCDEF", _portLetter) ) return 0;
	//проверка номера пина
	if ( _pinNumber > 15 ) return 0;
	
	//создаем новый элемент
	new_button = pushNewElement(ButtonsData);
	if ( !new_button ) return 0;
	
	//заполняем данные элемента
	new_button->port_idr_addr = getPortFromLetter(_portLetter);
	new_button->pin_addr = 1 << _pinNumber;
	new_button->_control_click_func = _click_func;
  new_button->event_code = _event_code;
  return 1;	
};

uint8_t ControlsRegNewValcoder(char _portLetterCCW, uint8_t _pinNumberCCW,
	                             char _portLetterCW, uint8_t _pinNumberCW,
															 void (*_ccw_func)(void), uint32_t _event_ccw_code,
															 void (*_cw_func)(void), uint32_t _event_cw_code)
{
	ControlsBaseStruct *new_valcoder = 0;
	//проверка буквы порта
	if ( !strchr("ABCDEF", _portLetterCCW) ) return 0;
	if ( !strchr("ABCDEF", _portLetterCW) ) return 0;
	//проверка номера пина
	if ( ( _pinNumberCCW > 15 ) || ( _pinNumberCW > 15 ) ) return 0;
	
	//создаем новый элемент CCW
	new_valcoder = pushNewElement(ValcoderData);
  if ( !new_valcoder ) return 0;
	//заполняем данные элемента
	new_valcoder->port_idr_addr = getPortFromLetter(_portLetterCCW);
	new_valcoder->pin_addr = 1 << _pinNumberCCW;
	new_valcoder->_control_click_func = _ccw_func;
  new_valcoder->event_code = _event_ccw_code;
	
	//создаем новый элемент CW
	new_valcoder = pushNewElement(ValcoderData);
  if ( !new_valcoder ) 
	{
		//удаляем предыдущий элемент и выходим
		popLastElement(ValcoderData);
		return 0;
	};
	//заполняем данные элемента
	new_valcoder->port_idr_addr = getPortFromLetter(_portLetterCW);
	new_valcoder->pin_addr = 1 << _pinNumberCW;
	new_valcoder->_control_click_func = _cw_func;
  new_valcoder->event_code = _event_cw_code;
	
  return 1;	
};

uint32_t ControlsUpdateEvents(uint32_t _params)
{
	//обработка событий и возврат результатов
	uint32_t ReturnedEvents = 0;
	ControlsBaseStruct *control_element = 0;
	ControlsBaseStruct *control_element_cw = 0;
	
	//опрос кнопок
	control_element = ButtonsData;
	while ( control_element )
	{
		//проверяем низкий уровень на пин
		if ( ( *control_element->port_idr_addr & control_element->pin_addr ) == 0 )
		{
			if ( !control_element->control_busy_flag )
			{
				//если кнопка нажата впервые
				//флаг
				control_element->control_busy_flag = 1;
				//событие по кнопке
				//если возвращаем код события
				if ( _params & UPDEVENTS_GETEVENTS ) ReturnedEvents |= control_element->event_code;
				//если вызываем обработчик события
				if ( ( _params & UPDEVENTS_HANDLERS_LAUNCH ) &&
             ( control_element->_control_click_func ) ) control_element->_control_click_func();
			} else
			{
				//кнопка срабатывает повторно/удерживается
				control_element->control_busy_flag = 1;
			};
		} else
		{
			//если кнопка отпущена, сбрасываем флаг
			if ( control_element->control_busy_flag )
			{
				control_element->control_busy_flag = 0;
			};				
		};
		//переключаемся на следующий элемент
		control_element = (ControlsBaseStruct*)control_element->next_control_element;
	};
	
	//опрос валкодеров
	control_element = ValcoderData;
	while ( control_element )
	{
		control_element_cw = (ControlsBaseStruct*)control_element->next_control_element;
		if ( !control_element_cw ) return VALCODER_ERROR;
		//опрос CCW - против часовой
		if ( ( ( *control_element->port_idr_addr & control_element->pin_addr ) == 0 ) &&
			   ( ( *control_element_cw->port_idr_addr & control_element_cw->pin_addr ) != 0 ) )
		{
			//если событие впервые
			if ( !control_element->control_busy_flag )
			{
				//устанавливаем флаг события
				control_element->control_busy_flag = 1;
				//если возвращаем код события
				if ( _params & UPDEVENTS_GETEVENTS ) ReturnedEvents |= control_element->event_code;
				//если вызываем обработчик события
				if ( ( _params & UPDEVENTS_HANDLERS_LAUNCH ) &&
             ( control_element->_control_click_func ) ) control_element->_control_click_func();				
			}
		} else
		//опрос CW - по часовой
		if ( ( ( *control_element->port_idr_addr & control_element->pin_addr ) != 0 ) &&
			   ( ( *control_element_cw->port_idr_addr & control_element_cw->pin_addr ) == 0 ) )
		{
			//если событие впервые
			if ( !control_element_cw->control_busy_flag )
			{
				//устанавливаем флаг события
				control_element_cw->control_busy_flag = 1;
				//если возвращаем код события
				if ( _params & UPDEVENTS_GETEVENTS ) ReturnedEvents |= control_element_cw->event_code;
				//если вызываем обработчик события
				if ( ( _params & UPDEVENTS_HANDLERS_LAUNCH ) &&
             ( control_element_cw->_control_click_func ) ) control_element_cw->_control_click_func();				
			}
		} else
		//опрос для сброса флагов
		if ( ( ( *control_element->port_idr_addr & control_element->pin_addr ) != 0 ) &&
			   ( ( *control_element_cw->port_idr_addr & control_element_cw->pin_addr ) != 0 ) )
		{
			//сброс флагов
			control_element->control_busy_flag = 0;
			control_element_cw->control_busy_flag = 0;
		};		
		//переключаемся на следующий элемент
		control_element = (ControlsBaseStruct*)control_element_cw->next_control_element;
	}
	
	return 1;
}

