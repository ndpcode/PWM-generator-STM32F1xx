
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

//��������� �������� ����������, ������ 24(32) ����, ������� 2 ��� ��������� ���������� �� flash
struct MainConfig
{
	unsigned isImmediateUpdate : 1;
	uint32_t freqPWM;
	uint32_t freqSignal;
	double powerK;
	uint8_t signalType;
} GenConfig;



uint8_t InitDefaults(void);
uint8_t DisplayInit(void);
SYS_EVENTS_DATA GenGetEventsFunc(void);
uint8_t MenuInit(void);



int main(void)
{
	//��������� RTC
	GenInitRTC();
	
	InitDefaults();
	//��������� ������������� ������� delay ���������� bsc_stm32_delay
	delayEnable();
	
	//����� ��, ��������� �������� ����������� � �����
	GenSystemSet();
	//��������� ������
	GenInitPorts();
	
	//��������� ������ � flash MCU
	if ( FlashAccessEnable() != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	//������ �������� �� flash
	
	//��������� ��������
	if ( GenInitTimers() != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//��������� �������
  if ( DisplayInit() != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//��������� ������
	if ( ControlsRegNewButton('B', 10, 0, EVENT_BUTTON1_CLICK, 0) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 11, 0, EVENT_BUTTON2_CLICK, 0) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 12, 0, EVENT_BUTTON3_CLICK, 0) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 13, 0, EVENT_BUTTON4_CLICK, 0) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 14, 0, EVENT_BUTTON5_CLICK, 0) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 15, 0, EVENT_BUTTON6_CLICK, 0) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('A', 3, 0,
		                        EVENT_VALCODER_BUTTON_CLICK,
	                          EVENT_VALCODER_BUTTON_PRESSED) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//��������� ���������
	if ( ControlsRegNewValcoder('A', 2,
                              'A', 4,
	                             0, EVENT_VALCODER_CCW,
                               0, EVENT_VALCODER_CW) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//�������� ����
	if ( MenuInit() != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//ErrorHandler(RESULT_FATAL_ERROR);
	
	//������ ���
	UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.powerK, GenConfig.signalType);
	
	while(1)
	{
		//���������� ���� � ��������� ����������
		MenuUpdate(GenMenu, getTimeS, getTimeMS);
	};
	
	ControlsDataClear();
}

uint8_t InitDefaults(void)
{
	//read from flash
	memset(&GenConfig, 0, sizeof(GenConfig));
	FlashReadData((uint8_t*)&GenConfig, sizeof(GenConfig));
	//�������� ������� ����������� ������
	if ( GenConfig.freqPWM == 0xFFFFFFFF )
	{
		//������ ��-���������
		GenConfig.freqPWM = 20000;
		GenConfig.freqSignal = 100;
		GenConfig.powerK = 100.0;
		GenConfig.signalType = 1;
		GenConfig.isImmediateUpdate = 0;
	};	
	return RESULT_OK;
}

uint8_t DisplayInit(void)
{
	//��������� ��������� �����
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
	 
	//������������� �������
	if ( HD44780DisplayInit(&Display) != HD44780_DISPLAY_OK )
	{
		//������� ������� �� ������
		HD44780DriverDestroy(&Display.portStruct);
		//�������
		return RESULT_ERROR;
	};
	
	//������� �������
	HD44780DisplayClear(&Display);
	
	return RESULT_OK;
}

SYS_EVENTS_DATA GenGetEventsFunc(void)
{
	//����� �������
	return ControlsUpdateEvents(UPDEVENTS_GETEVENTS | UPDEVENTS_HANDLERS_LAUNCH);
}

uint8_t MenuInit(void)
{
	//�������� ���� � ���������� 1 ������
	GenMenu = MenuCreate(MENU_DRAW_FPS, MENU_EVENTS_FPS, GenGetEventsFunc, MenuTransitionDraw, MENU_TRANS_TIME, Menu1Draw, Menu1Events);
  if ( !GenMenu ) return RESULT_ERROR;
	//��������� ������ ����	
	MenuAddNextItem(GenMenu, Menu2Draw, Menu2Events); 
  MenuAddNextItem(GenMenu, Menu3Draw, Menu3Events);
  MenuAddNextItem(GenMenu, Menu4Draw, Menu4Events);
  MenuAddNextItem(GenMenu, Menu5Draw, Menu5Events);
  MenuAddNextItem(GenMenu, Menu6Draw, Menu6Events);	
  MenuAddNextItem(GenMenu, Menu7Draw, Menu7Events);
	MenuAddSubItem(GenMenu, MenuSaveDraw, 0);
	return RESULT_OK;
}

void HardFault_Handler(void)
{
	ErrorHandler(RESULT_IO_ERROR);
}

