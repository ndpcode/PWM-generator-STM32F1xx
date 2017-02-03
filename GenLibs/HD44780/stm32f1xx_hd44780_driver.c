
#include "stm32f1xx_hd44780_driver.h"

unsigned char HD44780DriverSetPin(HD44780_PORT_STRUCT *_portStruct,
	                                HD44780_DISPLAY_PIN_NAME _pinName,
                                  HD44780_PIN_DIRECTION _pinDirection)
{
	//проверка входных данных
	if ( !_portStruct ) return HD44780_DRIVER_ERROR;
	if ( ( _pinName > 10 ) || ( _pinName < 0 ) ) return HD44780_DRIVER_ERROR;
	if ( !_portStruct->DisplayPort[_pinName].portAddress ) return HD44780_DRIVER_ERROR;
	if ( ( _portStruct->DisplayPort[_pinName].pinIndex > 15 ) ||
       ( _portStruct->DisplayPort[_pinName].pinIndex < 0 ) ) return HD44780_DRIVER_ERROR;
	if ( ( _pinDirection > 2 ) || ( _pinDirection < 0 ) ) return HD44780_DRIVER_ERROR;
	
	//настройка пина
	switch ( _pinDirection )
	{
		case HD44780_PIN_DIRECTION_INPUT:
      *(unsigned int*)(&_portStruct->DisplayPort[_pinName].portAddress->CRL +
				                  (_portStruct->DisplayPort[_pinName].pinIndex/8) ) &=
			~(0x0F << ( _portStruct->DisplayPort[_pinName].pinIndex - (_portStruct->DisplayPort[_pinName].pinIndex/8)*8 )*4 );			
		break;
		
		case HD44780_PIN_DIRECTION_OUTPUT:
      *(unsigned int*)(&_portStruct->DisplayPort[_pinName].portAddress->CRL +
				                  (_portStruct->DisplayPort[_pinName].pinIndex/8) ) &=
			~(0x05 << ( _portStruct->DisplayPort[_pinName].pinIndex - (_portStruct->DisplayPort[_pinName].pinIndex/8)*8 )*4 );
		break;
		
		case HD44780_PIN_DIRECTION_FLOATING:
      *(unsigned int*)(&_portStruct->DisplayPort[_pinName].portAddress->CRL +
				                  (_portStruct->DisplayPort[_pinName].pinIndex/8) ) &=
			~(0x04 << ( _portStruct->DisplayPort[_pinName].pinIndex - (_portStruct->DisplayPort[_pinName].pinIndex/8)*8 )*4 );
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
	if ( ( _pinName > 10 ) || ( _pinName < 0 ) ) return HD44780_DRIVER_ERROR;
	if ( !_portStruct->DisplayPort[_pinName].portAddress ) return HD44780_DRIVER_ERROR;
	if ( ( _portStruct->DisplayPort[_pinName].pinIndex > 15 ) ||
       ( _portStruct->DisplayPort[_pinName].pinIndex < 0 ) ) return HD44780_DRIVER_ERROR;
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
	if ( ( _pinName > 10 ) || ( _pinName < 0 ) ) return HD44780_DRIVER_ERROR;
	if ( !_portStruct->DisplayPort[_pinName].portAddress ) return HD44780_DRIVER_ERROR;
	if ( ( _portStruct->DisplayPort[_pinName].pinIndex > 15 ) ||
       ( _portStruct->DisplayPort[_pinName].pinIndex < 0 ) ) return HD44780_DRIVER_ERROR;
	
	//считываем уровень с пин
	return ( (_portStruct->DisplayPort[_pinName].portAddress->IDR & (1 << _portStruct->DisplayPort[_pinName].pinIndex)) != 0);
}

