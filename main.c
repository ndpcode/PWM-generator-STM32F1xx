
#include "settings.h"
#include "bsc_stm32_delay.h"
#include "gen_system.h"
#include "gen_rtc.h"
#include "gen_flash.h"
#include "gen_ports.h"
#include "gen_timers.h"
#include "hd44780display.h"
#include "bsc_controls.h"
#include "bsc_tree_menu.h"
#include "gen_menu.h"
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#define getTimeMS (uint32_t)( ( (RTC->CNTH << 16 ) | RTC->CNTL ) % 1000 )
#define getTimeS (uint32_t)( ( (RTC->CNTH << 16 ) | RTC->CNTL ) % 60000 ) / 1000

HD44780_DISPLAY_STRUCT Display;
TREE_MENU *GenMenu;

uint16_t MenuOKItemID;

//структура основных параметров, размер 24(32) байт, кратный 2 для упрощения сохранения на flash
struct MainConfig
{
	unsigned isImmediateUpdate : 1;
	int32_t freqPWM;
	int32_t freqSignal;
	int32_t powerK;
	int32_t centerK;
	int32_t pwmMinPulseLengthInNS;
	int32_t pwmDeadTimeInNS;
	uint8_t signalType;
} GenConfig;



uint8_t InitDefaults(void);
uint8_t DisplayInit(void);
SYS_EVENTS_DATA GenGetEventsFunc(void);
uint8_t MenuInit(void);



int main(void)
{
	//настройка RTC
	GenInitRTC();
	
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
	if ( ControlsRegNewButton('B', 10, 0, EVENT_BUTTON1_CLICK, 0) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 11, 0, EVENT_BUTTON2_CLICK, 0) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 12, 0, EVENT_BUTTON3_CLICK, 0) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 13, 0, EVENT_BUTTON4_CLICK, 0) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 14, 0, EVENT_BUTTON5_CLICK, 0) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 15, 0, EVENT_BUTTON6_CLICK, 0) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('A', 3, 0,
		                        EVENT_VALCODER_BUTTON_CLICK,
	                          EVENT_VALCODER_BUTTON_PRESSED) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//инициация валкодера
	if ( ControlsRegNewValcoder('A', 2,
                              'A', 4,
	                             0, EVENT_VALCODER_CCW,
                               0, EVENT_VALCODER_CW) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//создание меню
	if ( MenuInit() != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//запуск ШИМ
	UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.powerK, GenConfig.centerK,
               GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
	
	//светодиоды для индикации
	if ( GenConfig.isImmediateUpdate ) LED_BLUE_ON; else LED_BLUE_OFF;
	if ( GenConfig.signalType == 2 ) LED_GREEN_ON; else LED_GREEN_OFF;
	
	while(1)
	{
		//обновление меню и элементов управления
		MenuUpdate(GenMenu, getTimeS, getTimeMS);
	};
	
	ControlsDataClear();
}

uint8_t InitDefaults(void)
{
	//read from flash
	memset(&GenConfig, 0, sizeof(GenConfig));
	FlashReadData((uint8_t*)&GenConfig, sizeof(GenConfig));
	//проверка наличия сохраненных данных
	if ( GenConfig.freqPWM == 0xFFFFFFFF )
	{
		//данные по-умолчанию
		GenConfig.isImmediateUpdate = defaultUpdateType;
		GenConfig.freqPWM= defaultFreqPWM;
		GenConfig.freqSignal = defaultFreqSignal;
		GenConfig.powerK = defaultPowerK;
		GenConfig.centerK = defaultSignalCenter;
		GenConfig.pwmMinPulseLengthInNS = defaultTransistorsMinTime;
		GenConfig.pwmDeadTimeInNS = defaultTransistorsDeadTime;
		GenConfig.signalType = defaultSignalType;
	};	
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

SYS_EVENTS_DATA GenGetEventsFunc(void)
{
	//опрос событий
	return ControlsUpdateEvents(UPDEVENTS_GETEVENTS | UPDEVENTS_HANDLERS_LAUNCH);
}

uint8_t MenuInit(void)
{
	uint16_t varIndex = 0;
	//создание меню и добавление 1 пункта
	GenMenu = MenuCreate(MENU_DRAW_FPS, MENU_EVENTS_FPS, GenGetEventsFunc, MenuTransitionDraw, MENU_TRANS_TIME, MenuHiDraw, 0);
  if ( !GenMenu ) return RESULT_ERROR;
	//добавляем пункты меню
	//основное меню 1
	MenuAddNextItem(GenMenu, 0, Menu1_StartMenuDraw, Menu1_StartMenuEvents);
	//основное меню 2
	varIndex = MenuAddNextItem(GenMenu, 0, Menu2_MainMenuDraw, Menu2_MainMenuEvents);
	    //подменю 1 к меню 2
      MenuAddSubItem(GenMenu, varIndex, Menu2_SubMenu1_ChangePWMFreqDraw, Menu2_SubMenu1_ChangePWMFreqEvents);
	    //подменю 2 к меню 2
	    MenuAddNextItem(GenMenu, 0, Menu2_SubMenu2_ChangeSignalFreqDraw, Menu2_SubMenu2_ChangeSignalFreqEvents);
	    //подменю 3 к меню 2
	    MenuAddNextItem(GenMenu, 0, Menu2_SubMenu3_ChangeSignalPowerDraw, Menu2_SubMenu3_ChangeSignalPowerEvents);
	    //подменю 4 к меню 2
	    MenuAddNextItem(GenMenu, 0, Menu2_SubMenu4_ChangeSignalCenterDraw, Menu2_SubMenu4_ChangeSignalCenterEvents);
	//возврат к основному меню и добавление основного меню 3
	varIndex = MenuAddNextItem(GenMenu, varIndex, Menu3_ExtraMenuDraw, Menu3_ExtraMenuEvents);
	    //подменю 1 к меню 3
      MenuAddSubItem(GenMenu, varIndex, Menu3_SubMenu1_ChangeDeadTimeDraw, Menu3_SubMenu1_ChangeDeadTimeEvents);	
	    //подменю 2 к меню 3
	    MenuAddNextItem(GenMenu, 0, Menu3_SubMenu2_ChangeMinPulseTimeDraw, Menu3_SubMenu2_ChangeMinPulseTimeEvents);
	    //подменю 3 к меню 3
	    MenuAddNextItem(GenMenu, 0, Menu3_SubMenu3_ChangeSignalTypeDraw, Menu3_SubMenu3_ChangeSignalTypeEvents);
	    //подменю 4 к меню 3
	    MenuAddNextItem(GenMenu, 0, Menu3_SubMenu4_ChangeUpdateTypeDraw, Menu3_SubMenu4_ChangeUpdateTypeEvents);			
	//возврат к основному меню и добавление основного меню 4
	varIndex = MenuAddNextItem(GenMenu, varIndex, Menu4_SaveMenuDraw, Menu4_SaveMenuEvents);
	    //подменю 1 к меню 4
			MenuAddSubItem(GenMenu, varIndex, Menu4_SubMenu1_SaveDialogDraw, 0);
	//добавляем меню OK
	MenuOKItemID = MenuAddNextItem(GenMenu, varIndex, MenuOKDraw, 0);
	return RESULT_OK;
}

void HardFault_Handler(void)
{
	ErrorHandler(RESULT_FATAL_ERROR);
}

