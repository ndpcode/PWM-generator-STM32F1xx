
#include "stm32f1xx_hd44780_driver.h"
#include <stdlib.h>
#include <string.h>

GPIO_TypeDef* getPortAddressFromLetter(char _portLetter)
{	
	//без проверок буквы порта
	//возвращаем адрес на порт
	switch ( _portLetter )
	{
		case 'A': return GPIOA;			
		case 'B': return GPIOB;			
		case 'C': return GPIOC;			
		case 'D': return GPIOD;			
	  case 'E': return GPIOE;			
		case 'F': return GPIOF;
	};
	return 0;
}

HD44780_PORT_STRUCT* HD44780DriverInit(unsigned char *portTemplate, unsigned char is4BitMode)
{
	HD44780_PORT_STRUCT *newPortStruct = 0;
	unsigned char i;
	//проверка входных данных
	if ( !portTemplate ) return 0;
	if ( ( is4BitMode != 0 ) && ( is4BitMode != 1 ) ) return 0;
	
	//выделение памяти и проверка
	newPortStruct = (HD44780_PORT_STRUCT*)malloc(sizeof(HD44780_PORT_STRUCT));
	if ( !newPortStruct ) return 0;
	//обнуляем память
	memset(newPortStruct, 0, sizeof(HD44780_PORT_STRUCT));
	
	//заполнение настроек драйвера
	for ( i = is4BitMode*4; i < 11; i++ )
	{
		newPortStruct->DisplayPort[i].portAddress = getPortAddressFromLetter(portTemplate[i*2]);
		newPortStruct->DisplayPort[i].pinIndex = portTemplate[i*2 + 1];
		if ( ( !newPortStruct->DisplayPort[i].portAddress ) ||
			   ( newPortStruct->DisplayPort[i].pinIndex > 15 ) )
		{
			//что-то пошло не так, очищаем память, выходим
			free(newPortStruct);
			return 0;			
		}
	};
	
	return newPortStruct;
}

unsigned char HD44780DriverDestroy(HD44780_PORT_STRUCT **driverStruct)
{
	//проверка входных данных
	if ( !driverStruct ) return HD44780_DRIVER_ERROR;
	//проверка значения указателя на данные
	if ( !*driverStruct ) return HD44780_DRIVER_ERROR;
	//очистка памяти данных
	free(*driverStruct);
	//очистка указателя на данные
	*driverStruct = 0;
	return HD44780_DRIVER_OK;	
}

unsigned char HD44780DriverSetPin(HD44780_PORT_STRUCT *_portStruct,
	                                HD44780_DISPLAY_PIN_NAME _pinName,
                                  HD44780_PIN_DIRECTION _pinDirection)
{
	//проверка входных данных
	if ( !_portStruct ) return HD44780_DRIVER_ERROR;
	if ( _pinName > 10 ) return HD44780_DRIVER_ERROR;
	if ( !_portStruct->DisplayPort[_pinName].portAddress ) return HD44780_DRIVER_ERROR;
	if ( _portStruct->DisplayPort[_pinName].pinIndex > 15 ) return HD44780_DRIVER_ERROR;
	if ( _pinDirection > 2 ) return HD44780_DRIVER_ERROR;
	
	//настройка пина
	switch ( _pinDirection )
	{
		case HD44780_PIN_DIRECTION_INPUT:
      *(unsigned int*)(&_portStruct->DisplayPort[_pinName].portAddress->CRL +
				                  (_portStruct->DisplayPort[_pinName].pinIndex/8) ) &=
			~(0x0F << ( _portStruct->DisplayPort[_pinName].pinIndex - (_portStruct->DisplayPort[_pinName].pinIndex/8)*8 )*4 );
      *(unsigned int*)(&_portStruct->DisplayPort[_pinName].portAddress->CRL +
				                  (_portStruct->DisplayPort[_pinName].pinIndex/8) ) |=
			(STM32F1XXPIN_DIRECTION_INPUT << ( _portStruct->DisplayPort[_pinName].pinIndex - (_portStruct->DisplayPort[_pinName].pinIndex/8)*8 )*4 );			
		break;
		
		case HD44780_PIN_DIRECTION_OUTPUT:
      *(unsigned int*)(&_portStruct->DisplayPort[_pinName].portAddress->CRL +
				                  (_portStruct->DisplayPort[_pinName].pinIndex/8) ) &=
			~(0x0F << ( _portStruct->DisplayPort[_pinName].pinIndex - (_portStruct->DisplayPort[_pinName].pinIndex/8)*8 )*4 );
      *(unsigned int*)(&_portStruct->DisplayPort[_pinName].portAddress->CRL +
				                  (_portStruct->DisplayPort[_pinName].pinIndex/8) ) |=
			(STM32F1XXPIN_DIRECTION_OUTPUT << ( _portStruct->DisplayPort[_pinName].pinIndex - (_portStruct->DisplayPort[_pinName].pinIndex/8)*8 )*4 );
		break;
		
		case HD44780_PIN_DIRECTION_FLOATING:
      *(unsigned int*)(&_portStruct->DisplayPort[_pinName].portAddress->CRL +
				                  (_portStruct->DisplayPort[_pinName].pinIndex/8) ) &=
			~(0x0F << ( _portStruct->DisplayPort[_pinName].pinIndex - (_portStruct->DisplayPort[_pinName].pinIndex/8)*8 )*4 );
      *(unsigned int*)(&_portStruct->DisplayPort[_pinName].portAddress->CRL +
				                  (_portStruct->DisplayPort[_pinName].pinIndex/8) ) |=
			(STM32F1XXPIN_DIRECTION_FLOATING << ( _portStruct->DisplayPort[_pinName].pinIndex - (_portStruct->DisplayPort[_pinName].pinIndex/8)*8 )*4 );
		break;
	};
	return HD44780_DRIVER_OK;	
}

unsigned char HD44780DriverWritePin(HD44780_PORT_STRUCT *_portStruct,
                                    HD44780_DISPLAY_PIN_NAME _pinName,
                                    unsigned char _logicLevel)
{
	//проверка входных данных
	if ( !_portStruct ) return HD44780_DRIVER_ERROR;
	if ( _pinName > 10 ) return HD44780_DRIVER_ERROR;
	if ( !_portStruct->DisplayPort[_pinName].portAddress ) return HD44780_DRIVER_ERROR;
	if ( _portStruct->DisplayPort[_pinName].pinIndex > 15 )  return HD44780_DRIVER_ERROR;
  if ( ( _logicLevel != 0 ) && ( _logicLevel != 1 ) )	return HD44780_DRIVER_ERROR;
	
	//меняем уровень на пин
	if ( _logicLevel )
	{
		_portStruct->DisplayPort[_pinName].portAddress->BSRR = (1 << _portStruct->DisplayPort[_pinName].pinIndex);
	} else
	{
		_portStruct->DisplayPort[_pinName].portAddress->BRR = (1 << _portStruct->DisplayPort[_pinName].pinIndex);
	};
  return HD44780_DRIVER_OK;	
}

unsigned char HD44780DriverReadPin(HD44780_PORT_STRUCT *_portStruct,
	                                 HD44780_DISPLAY_PIN_NAME _pinName)
{
	//проверка входных данных
	if ( !_portStruct ) return HD44780_DRIVER_ERROR;
	if ( _pinName > 10 ) return HD44780_DRIVER_ERROR;
	if ( !_portStruct->DisplayPort[_pinName].portAddress ) return HD44780_DRIVER_ERROR;
	if ( _portStruct->DisplayPort[_pinName].pinIndex > 15 ) return HD44780_DRIVER_ERROR;
	
	//считываем уровень с пин
	return ( (_portStruct->DisplayPort[_pinName].portAddress->IDR & (1 << _portStruct->DisplayPort[_pinName].pinIndex)) != 0);
}

