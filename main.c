
#include "settings.h"
#include "bsc_stm32_delay.h"
#include "gen_system.h"
#include "gen_flash.h"
#include "gen_ports.h"
#include "gen_timers.h"
#include "gen_controls.h"
#include "hd44780display.h"
//#include "gen_menu.h"
#include <limits.h>

HD44780_DISPLAY_STRUCT Display;

uint8_t InitDefaults(void);
uint8_t DisplayInit(void);
void valcoderbuttonclick(void)
{
	GPIOA->ODR = GPIOA->ODR ^ GPIO_ODR_ODR0;
}

int main(void)
{
	InitDefaults();
	//разрешаем использование функций delay библиотеки bsc_stm32_delay
	delayEnable();
	
	//старт МК, настройка тактовых генераторов и часов
	GenSystemSet();
	//настройка портов
	GenInitPorts();
	
	//разрешаем доступ к flash MCU
	if ( FlashAccessEnable() != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	//чтение настроек из flash
	
	//настройка таймеров
	if ( GenInitTimers() != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//инициация дисплея
  if ( DisplayInit() != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//инициация кнопок
	if ( ControlsRegNewButton('B', 10, 0, EVENT_BUTTON1_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 11, 0, EVENT_BUTTON2_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 12, 0, EVENT_BUTTON3_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 13, 0, EVENT_BUTTON4_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 14, 0, EVENT_BUTTON5_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 15, 0, EVENT_BUTTON6_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('A', 3, valcoderbuttonclick, EVENT_VALCODER_BUTTON_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//инициация валкодера
	if ( ControlsRegNewValcoder('A', 2,
                              'A', 4,
	                             0, EVENT_VALCODER_CCW,
                               0, EVENT_VALCODER_CW) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	HD44780DisplayWriteString(&Display, "Это ", 1, 3);
	HD44780DisplayWriteString(&Display, "X", 2, 13);
  HD44780DisplayWriteDouble(&Display, -8347.596, 3, 2, 3);

	//ErrorHandler(RESULT_FATAL_ERROR);
	
	while(1)
	{
		//опрос кнопок
		ControlsUpdateEvents(UPDEVENTS_GETEVENTS | UPDEVENTS_HANDLERS_LAUNCH);
	};
	
	ControlsDataClear();
}

uint8_t InitDefaults(void)
{
	return RESULT_OK;
}

uint8_t DisplayInit(void)
{
	//заполняем структуру порта
	unsigned char displayPortConfig[11][2] =
	{ 'X', 0, //HD44780_DRIVER_DATA_0 
    'X', 0, //HD44780_DRIVER_DATA_1
    'X', 0, //HD44780_DRIVER_DATA_2
    'X', 0, //HD44780_DRIVER_DATA_3
    'B', 4, //HD44780_DRIVER_DATA_4
    'B', 5, //HD44780_DRIVER_DATA_5
    'B', 6, //HD44780_DRIVER_DATA_6
    'B', 7, //HD44780_DRIVER_DATA_7
    'B', 9, //HD44780_DRIVER_ENABLE
    'B', 3, //HD44780_DRIVER_REG_SELECT
    'B', 8 //HD44780_DRIVER_READ_WRITE
	 };
	Display.portStruct = HD44780DriverInit(&displayPortConfig[0][0], 1);
	if ( !Display.portStruct ) return RESULT_ERROR;
	Display.delayUSFunc = delayUS;
	Display.displayEntryMode = HD44780_FLAG_ENTRYSHIFTOFF | HD44780_FLAG_ENTRYRIGHT;
	Display.displayOnOffControl = HD44780_FLAG_DISPLAYON | HD44780_FLAG_CURSOROFF | HD44780_FLAG_BLINKOFF;
	Display.displayFunctionSet = HD44780_FLAG_4BITMODE | HD44780_FLAG_2LINE | HD44780_FLAG_5x10DOTS;
	 
	//инициализация дисплея
	if ( HD44780DisplayInit(&Display) != HD44780_DISPLAY_OK )
	{
		//удаляем драйвер из памяти
		HD44780DriverDestroy(&Display.portStruct);
		//выходим
		return RESULT_ERROR;
	};
	
	//очистка дисплея
	HD44780DisplayClear(&Display);
	
	return RESULT_OK;
}

void HardFault_Handler(void)
{
	ErrorHandler(RESULT_IO_ERROR);
}

