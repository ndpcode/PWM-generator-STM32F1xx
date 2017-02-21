
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

//��������� �������� ����������, ������ 24(32) ����, ������� 2 ��� ��������� ���������� �� flash
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
	
	//������ ���
	UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.powerK, GenConfig.centerK,
               GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
	
	//���������� ��� ���������
	if ( GenConfig.isImmediateUpdate ) LED_BLUE_ON; else LED_BLUE_OFF;
	if ( GenConfig.signalType == 2 ) LED_GREEN_ON; else LED_GREEN_OFF;
	
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
	uint16_t varIndex = 0;
	//�������� ���� � ���������� 1 ������
	GenMenu = MenuCreate(MENU_DRAW_FPS, MENU_EVENTS_FPS, GenGetEventsFunc, MenuTransitionDraw, MENU_TRANS_TIME, MenuHiDraw, 0);
  if ( !GenMenu ) return RESULT_ERROR;
	//��������� ������ ����
	//�������� ���� 1
	MenuAddNextItem(GenMenu, 0, Menu1_StartMenuDraw, Menu1_StartMenuEvents);
	//�������� ���� 2
	varIndex = MenuAddNextItem(GenMenu, 0, Menu2_MainMenuDraw, Menu2_MainMenuEvents);
	    //������� 1 � ���� 2
      MenuAddSubItem(GenMenu, varIndex, Menu2_SubMenu1_ChangePWMFreqDraw, Menu2_SubMenu1_ChangePWMFreqEvents);
	    //������� 2 � ���� 2
	    MenuAddNextItem(GenMenu, 0, Menu2_SubMenu2_ChangeSignalFreqDraw, Menu2_SubMenu2_ChangeSignalFreqEvents);
	    //������� 3 � ���� 2
	    MenuAddNextItem(GenMenu, 0, Menu2_SubMenu3_ChangeSignalPowerDraw, Menu2_SubMenu3_ChangeSignalPowerEvents);
	    //������� 4 � ���� 2
	    MenuAddNextItem(GenMenu, 0, Menu2_SubMenu4_ChangeSignalCenterDraw, Menu2_SubMenu4_ChangeSignalCenterEvents);
	//������� � ��������� ���� � ���������� ��������� ���� 3
	varIndex = MenuAddNextItem(GenMenu, varIndex, Menu3_ExtraMenuDraw, Menu3_ExtraMenuEvents);
	    //������� 1 � ���� 3
      MenuAddSubItem(GenMenu, varIndex, Menu3_SubMenu1_ChangeDeadTimeDraw, Menu3_SubMenu1_ChangeDeadTimeEvents);	
	    //������� 2 � ���� 3
	    MenuAddNextItem(GenMenu, 0, Menu3_SubMenu2_ChangeMinPulseTimeDraw, Menu3_SubMenu2_ChangeMinPulseTimeEvents);
	    //������� 3 � ���� 3
	    MenuAddNextItem(GenMenu, 0, Menu3_SubMenu3_ChangeSignalTypeDraw, Menu3_SubMenu3_ChangeSignalTypeEvents);
	    //������� 4 � ���� 3
	    MenuAddNextItem(GenMenu, 0, Menu3_SubMenu4_ChangeUpdateTypeDraw, Menu3_SubMenu4_ChangeUpdateTypeEvents);			
	//������� � ��������� ���� � ���������� ��������� ���� 4
	varIndex = MenuAddNextItem(GenMenu, varIndex, Menu4_SaveMenuDraw, Menu4_SaveMenuEvents);
	    //������� 1 � ���� 4
			MenuAddSubItem(GenMenu, varIndex, Menu4_SubMenu1_SaveDialogDraw, 0);
	//��������� ���� OK
	MenuOKItemID = MenuAddNextItem(GenMenu, varIndex, MenuOKDraw, 0);
	return RESULT_OK;
}

void HardFault_Handler(void)
{
	ErrorHandler(RESULT_FATAL_ERROR);
}

