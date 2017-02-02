
#include "settings.h"
#include "gen_system.h"
#include "gen_flash.h"
#include "gen_ports.h"
#include "gen_timers.h"
#include "gen_controls.h"
#include "bsc_stm32_delay.h"
#include <limits.h>

uint8_t InitDefaults(void);

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
	
	//инициация кнопок
	if ( ControlsRegNewButton('B', 10, 0, EVENT_BUTTON1_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 11, 0, EVENT_BUTTON2_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 12, 0, EVENT_BUTTON3_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 13, 0, EVENT_BUTTON4_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 14, 0, EVENT_BUTTON5_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 15, 0, EVENT_BUTTON6_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//инициация валкодера
	if ( ControlsRegNewValcoder('A', 2,
                              'A', 4,
	                             0, EVENT_VALCODER_CCW,
                               0, EVENT_VALCODER_CW) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
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
