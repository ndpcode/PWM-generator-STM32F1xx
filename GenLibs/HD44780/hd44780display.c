
#include "hd44780display.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//****************************************************************************************************************//
//****************************************************************************************************************//
//Блок внутренних функций, упрощающих работу с контроллером дисплея.
//Внутри этих функций отсутствуют проверки на ошибки и совпадение типов,
//проверка должна выполняться предварительно.
//****************************************************************************************************************//
//****************************************************************************************************************//

static unsigned char HD44780DisplayCheckConfig(HD44780_DISPLAY_STRUCT *displayStruct)
{
	if ( !displayStruct ) return HD44780_DISPLAY_ERROR;
	if ( !displayStruct->portStruct ) return HD44780_DISPLAY_CONFIG_ERROR;
	if ( !displayStruct->delayUSFunc ) return HD44780_DISPLAY_CONFIG_ERROR;	
	return HD44780_DISPLAY_OK;
}

static unsigned char HD44780DisplayResetDataPins(HD44780_DISPLAY_STRUCT *displayStruct)
{
	unsigned char i;
	for ( i = 0; i < 8; i++ )
	{
		HD44780DriverWritePin(displayStruct->portStruct, (HD44780_DISPLAY_PIN_NAME)i, 0);		
	};
	return HD44780_DISPLAY_OK;
}

static unsigned char HD44780DisplayWriteHalfByte(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char halfByte)
{
	unsigned char i;
	for ( i = 4; i < 8; i++ )
	{
		HD44780DriverWritePin(displayStruct->portStruct, (HD44780_DISPLAY_PIN_NAME)i, ( halfByte >> ( i - 4 ) ) & 0x01);
	};
	return HD44780_DISPLAY_OK;
}

static unsigned char HD44780DisplayPulse(HD44780_DISPLAY_STRUCT *displayStruct)
{
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(2); //пауза 2 мкс
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //пауза 2 мкс
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(2); //пауза 2 мкс
	return HD44780_DISPLAY_OK;
}

static unsigned char HD44780DisplayWaitBusyFlag(HD44780_DISPLAY_STRUCT *displayStruct)
{
	HD44780DisplayResetDataPins(displayStruct); //ALL DBx  = 0
	HD44780DriverSetPin(displayStruct->portStruct, HD44780_DRIVER_DATA_7, HD44780_PIN_DIRECTION_INPUT); //DB7 - input
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_READ_WRITE, 1); //RW = 1 (read data)
	do
	{
		HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	  displayStruct->delayUSFunc(1); //пауза 1 мкс
		HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
		displayStruct->delayUSFunc(1); //пауза 1 мкс
	} while ( HD44780DriverReadPin(displayStruct->portStruct, HD44780_DRIVER_DATA_7) ); //Wait BS = 0
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_READ_WRITE, 0); //RW = 0 (write data)
  HD44780DriverSetPin(displayStruct->portStruct, HD44780_DRIVER_DATA_7, HD44780_PIN_DIRECTION_OUTPUT); //DB7 - output
	return HD44780_DISPLAY_OK;
}

static unsigned char GetNumberOfDigits(int _number)
{
	return ( _number == 0 ) ? 1 : (unsigned char) ceil( log10( abs(_number) + 0.5 ) );
}

//****************************************************************************************************************//
//****************************************************************************************************************//
//Блок основных функций для работы с контроллером дисплея.
//Эти функции доступны для вызовов вне библиотеки.
//****************************************************************************************************************//
//****************************************************************************************************************//

unsigned char HD44780DisplayInit(HD44780_DISPLAY_STRUCT *displayStruct)
{
	//проверка входных данных
	if ( HD44780DisplayCheckConfig(displayStruct) != HD44780_DISPLAY_OK ) return HD44780_DISPLAY_CONFIG_ERROR;
	
	//проверка флага BS
	//HD44780DisplayWaitBusyFlag(displayStruct);
	
	//инициализация
  HD44780DisplayResetDataPins(displayStruct); //ALL DBx  = 0
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(50000); //пауза 50000 мкс
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_READ_WRITE, 0); //RW = 0
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_REG_SELECT, 0); //A0 = 0
	
	HD44780DisplayWriteHalfByte(displayStruct, 0x03);
	displayStruct->delayUSFunc(2); //пауза 2 мкс
	
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //пауза 2 мкс
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(200); //пауза 200 мкс
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //пауза 2 мкс
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(200); //пауза 200 мкс
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //пауза 2 мкс
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(200); //пауза 200 мкс
	
  HD44780DisplayResetDataPins(displayStruct); //ALL DBx  = 0
	HD44780DisplayWriteHalfByte(displayStruct, 0x02);
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //пауза 2 мкс
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(200); //пауза 200 мкс
	
	HD44780DisplaySendByte(displayStruct, HD44780_CMD_FUNCTIONSET | displayStruct->displayFunctionSet, HD44780_DISPLAY_COMMAND);
	HD44780DisplaySendByte(displayStruct, HD44780_CMD_DISPLAYCONTROL | displayStruct->displayOnOffControl, HD44780_DISPLAY_COMMAND);
	HD44780DisplaySendByte(displayStruct, HD44780_CMD_ENTRYMODESET | displayStruct->displayEntryMode, HD44780_DISPLAY_COMMAND);
	HD44780DisplayClear(displayStruct);
	
	return HD44780_DISPLAY_OK;
}

unsigned char HD44780DisplayClear(HD44780_DISPLAY_STRUCT *displayStruct)
{
	if ( HD44780DisplaySendByte(displayStruct, 0x01, HD44780_DISPLAY_COMMAND) != HD44780_DISPLAY_OK ) return HD44780_DISPLAY_CONFIG_ERROR;
	displayStruct->delayUSFunc(5000); //пауза 5000 мкс
	return HD44780_DISPLAY_OK;
}

unsigned char HD44780DisplaySendByte(HD44780_DISPLAY_STRUCT *displayStruct,
                                                 unsigned char displayData,
																				  HD44780_DISPLAY_BYTE_TYPE isData)
{
	//проверка входных данных
	if ( HD44780DisplayCheckConfig(displayStruct) != HD44780_DISPLAY_OK ) return HD44780_DISPLAY_CONFIG_ERROR;
	if ( ( isData != 0 ) && ( isData != 1 ) ) return HD44780_DISPLAY_ERROR;
	//отправляем сигнал
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_READ_WRITE, 0); //RW = 0
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_REG_SELECT, isData); //A0 = 0(команда) или A0 = 1(данные)
	HD44780DisplayResetDataPins(displayStruct); //ALL DBx  = 0
	HD44780DisplayWriteHalfByte(displayStruct, (displayData >> 4 ) & 0x0F); //пишем старшую половину байта
  displayStruct->delayUSFunc(2); //пауза 2 мкс
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //пауза 2 мкс
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(2); //пауза 2 мкс
	HD44780DisplayResetDataPins(displayStruct); //ALL DBx  = 0
	HD44780DisplayWriteHalfByte(displayStruct, displayData & 0x0F); //пишем младшую половину байта
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //пауза 2 мкс
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(40); //пауза 40 мкс
	//HD44780DisplayWaitBusyFlag(displayStruct);
	return HD44780_DISPLAY_OK;
}

unsigned char HD44780DisplayMoveCursor(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char displayRow,
	                                     unsigned char displayColumn)
{
	unsigned char row_offsets[] = { 0x00, 0x40, 0x10, 0x50 };
	//проверка входных данных
	if ( HD44780DisplayCheckConfig(displayStruct) != HD44780_DISPLAY_OK ) return HD44780_DISPLAY_CONFIG_ERROR;
	if ( ( displayRow > 4 ) || ( !displayRow ) ) return HD44780_DISPLAY_ERROR;
	return HD44780DisplaySendByte(displayStruct,
                                HD44780_CMD_SETDDRAMADDR | (row_offsets[displayRow-1] + displayColumn),
	                              HD44780_DISPLAY_COMMAND);
}

unsigned char HD44780DisplayWriteChar(HD44780_DISPLAY_STRUCT *displayStruct, char displayChar,
																			unsigned char displayRow, unsigned char displayColumn)
{
	//перемещение курсора
	if ( HD44780DisplayMoveCursor(displayStruct, displayRow, displayColumn) != HD44780_DISPLAY_OK ) return HD44780_DISPLAY_ERROR;
	//отправка данных
	if ( HD44780DisplaySendByte(displayStruct, displayChar, HD44780_DISPLAY_DATA) != HD44780_DISPLAY_OK ) return HD44780_DISPLAY_ERROR; 
	return HD44780_DISPLAY_OK;
}

unsigned char HD44780DisplayWriteString(HD44780_DISPLAY_STRUCT *displayStruct, const char *displayString,
																			  unsigned char displayRow, unsigned char displayColumn)
{
	//проверка входных данных
	if ( !displayString ) return HD44780_DISPLAY_CONFIG_ERROR;
	
	//перемещение курсора
	if ( HD44780DisplayMoveCursor(displayStruct, displayRow, displayColumn) != HD44780_DISPLAY_OK ) return HD44780_DISPLAY_ERROR;	
	//пишем данные
	while ( *displayString )
		if ( HD44780DisplaySendByte(displayStruct, *(displayString++), HD44780_DISPLAY_DATA) != HD44780_DISPLAY_OK )
			return HD44780_DISPLAY_ERROR; 
	return HD44780_DISPLAY_OK;
}

unsigned char HD44780DisplayWriteInt32(HD44780_DISPLAY_STRUCT *displayStruct, int displayInt,
                                       unsigned char displayRow, unsigned char displayColumn)
{
	//преобразование числа в строку
	char displayString[12];	
	snprintf(displayString, 12, "%d", displayInt);
	//пишем строку
	return HD44780DisplayWriteString(displayStruct, displayString, displayRow, displayColumn);
}

unsigned char HD44780DisplayWriteDouble(HD44780_DISPLAY_STRUCT *displayStruct, double displayDouble, char doubleAccuracy,
                                        unsigned char displayRow, unsigned char displayColumn)
{
	//преобразование числа в строку
	char displayString[33];
	char _a[6] = "%.";
	char _b[] = "lf";
	doubleAccuracy += '0';
	strncat(_a, &doubleAccuracy, 2);
	strncat(_a, _b, 2);
	snprintf(displayString, 33, _a, displayDouble);
	//пишем строку
	return HD44780DisplayWriteString(displayStruct, displayString, displayRow, displayColumn);
}

unsigned char HD44780DisplayDisplayOnOff(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char isOn)
{
	//отправка команды
	if ( !displayStruct ) return HD44780_DISPLAY_ERROR;
	displayStruct->displayOnOffControl &= ~HD44780_FLAG_DISPLAYON;
	displayStruct->displayOnOffControl |= (isOn == 0) ? HD44780_FLAG_DISPLAYOFF : HD44780_FLAG_DISPLAYON;
	return HD44780DisplaySendByte(displayStruct,
                                HD44780_CMD_DISPLAYCONTROL | displayStruct->displayOnOffControl,
	                              HD44780_DISPLAY_COMMAND);
}

unsigned char HD44780DisplaySetCursorVisible(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char isCursor)
{
	//отправка команды
	if ( !displayStruct ) return HD44780_DISPLAY_ERROR;
	displayStruct->displayOnOffControl &= ~HD44780_FLAG_CURSORON;
	displayStruct->displayOnOffControl |= (isCursor == 0) ? HD44780_FLAG_CURSOROFF : HD44780_FLAG_CURSORON;
	return HD44780DisplaySendByte(displayStruct,
                                HD44780_CMD_DISPLAYCONTROL | displayStruct->displayOnOffControl,
	                              HD44780_DISPLAY_COMMAND);
}

unsigned char HD44780DisplaySetBlinkVisible(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char isBlink)
{
	//отправка команды
	if ( !displayStruct ) return HD44780_DISPLAY_ERROR;
	displayStruct->displayOnOffControl &= ~HD44780_FLAG_BLINKON;
	displayStruct->displayOnOffControl |= (isBlink == 0) ? HD44780_FLAG_BLINKOFF : HD44780_FLAG_BLINKON;
	return HD44780DisplaySendByte(displayStruct,
                                HD44780_CMD_DISPLAYCONTROL | displayStruct->displayOnOffControl,
	                              HD44780_DISPLAY_COMMAND);
}

unsigned char HD44780DisplaySetScrollDirection(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char isLeftToRight)
{
	//отправка команды
	if ( !displayStruct ) return HD44780_DISPLAY_ERROR;
	displayStruct->displayEntryMode &= ~HD44780_FLAG_ENTRYRIGHT;
	displayStruct->displayEntryMode |= (isLeftToRight == 0) ? HD44780_FLAG_ENTRYLEFT : HD44780_FLAG_ENTRYRIGHT;
	return HD44780DisplaySendByte(displayStruct,
                                HD44780_CMD_ENTRYMODESET | displayStruct->displayEntryMode,
	                              HD44780_DISPLAY_COMMAND);
}

unsigned char HD44780DisplaySetAutoscroll(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char isAutoscroll)
{
	//отправка команды
	if ( !displayStruct ) return HD44780_DISPLAY_ERROR;
	displayStruct->displayEntryMode &= ~HD44780_FLAG_ENTRYSHIFTON;
	displayStruct->displayEntryMode |= (isAutoscroll == 0) ? HD44780_FLAG_ENTRYSHIFTOFF : HD44780_FLAG_ENTRYSHIFTON;
	return HD44780DisplaySendByte(displayStruct,
                                HD44780_CMD_ENTRYMODESET | displayStruct->displayEntryMode,
	                              HD44780_DISPLAY_COMMAND);
}

