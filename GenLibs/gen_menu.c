
#include "settings.h"
#include "gen_menu.h"
#include "gen_system.h"
#include "gen_timers.h"
#include "hd44780display.h"
#include "bsc_tree_menu.h"
#include "bsc_stm32_delay.h"
#include "gen_flash.h"
#include "bsc_anim_ctrls.h"
#include "localization.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

//значения дельты приращения частот при повороте ручки валкодера
#define DeltaValMinValue 1
#define DeltaValMaxValue 1000

extern HD44780_DISPLAY_STRUCT Display;
extern TREE_MENU *GenMenu;
extern uint16_t MenuOKItemID;
extern uint16_t Menu2ItemID;
extern char **locLanguageData;
extern struct MainConfig
{
	unsigned isImmediateUpdate : 1;
	unsigned ShowFreqType : 3;
	unsigned languageId : 4;
	uint8_t signalType;
	double freqPWM;
	double freqSignal;
	int32_t powerK;
	int32_t centerK;
	int32_t pwmMinPulseLengthInNS;
	int32_t pwmDeadTimeInNS;
	uint16_t timerPrescaler;
	uint16_t timerARR;
	uint16_t timerStepsCCR;
} GenConfig;

char Line1Buffer[17] = "                ";
char Line2Buffer[17] = "                ";

ANIM_VAR_NUMBER *valMain;
ANIM_VAR_NUMBER *valIncSize;
int32_t incSize;

//глобальные переменные для хранения промежуточных значений
//размещаем сразу, чтобы не тратить время на выделение памяти
//указатель на изменяемое число (на 1 строке или на 2)
uint8_t cursorNumN;
char displayString[17];
//направление перехода меню влево(+) - 2, вправо(-) - 1
uint8_t menuTransDirection = 0;
int32_t bufferVal = 0;
int32_t bufferVarK = 0;
double bufferValDouble = 0;
int16_t menu_iteration_int1 = 0;
int16_t menu_iteration_int2 = 0;
uint16_t menu_iteration_uint1 = 0;
uint16_t menu_iteration_uint2 = 0;
int32_t menu_int32 = 0;
char Line1BufferNew[17] = "                ";
char Line2BufferNew[17] = "                ";
uint16_t LastMenuItemID = 0;

//*****************************************************************************************************************************//
//*****************************************************************************************************************************//
// additional
//*****************************************************************************************************************************//
//*****************************************************************************************************************************//

void GenChangeMenu(void)
{
	//find and delete
	TREE_MENU_ITEM *_menu_item = MenuGetItemById(GenMenu, Menu2ItemID);
	if ( !_menu_item ) return;
	if ( !_menu_item->MenuChild ) return;
	MenuDeleteItem(GenMenu, ((TREE_MENU_ITEM*)_menu_item->MenuChild)->MenuItemId);
	//create new
	if ( ( GenConfig.ShowFreqType == ShowFreqTypeNormal ) || ( GenConfig.ShowFreqType == ShowFreqTypeActualFreq ) )
	{
		//подменю 1 к меню 2
    MenuAddSubItem(GenMenu, Menu2ItemID, Menu2_SubMenu1_ChangePWMFreqDraw, Menu2_SubMenu1_ChangePWMFreqEvents);
		if ( GenConfig.signalType != signalSquare )
		{
			//подменю 2 к меню 2
	    MenuAddNextItem(GenMenu, 0, Menu2_SubMenu2_ChangeSignalFreqDraw, Menu2_SubMenu2_ChangeSignalFreqEvents);
		}
	  //подменю 3 к меню 2
	  MenuAddNextItem(GenMenu, 0, Menu2_SubMenu3_ChangeSignalPowerDraw, Menu2_SubMenu3_ChangeSignalPowerEvents);
		if ( GenConfig.signalType != signalSquare )
		{
			//подменю 4 к меню 2
	    MenuAddNextItem(GenMenu, 0, Menu2_SubMenu4_ChangeSignalCenterDraw, Menu2_SubMenu4_ChangeSignalCenterEvents);			
		}
	}
	if ( GenConfig.ShowFreqType == ShowFreqTypeDirectControl )
	{
		//подменю 1 к меню 2
    MenuAddSubItem(GenMenu, Menu2ItemID, Menu2_SubMenu1_Alt_ChangePrescalerDraw, Menu2_SubMenu1_Alt_ChangePrescalerEvents);
	  //подменю 2 к меню 2
	  MenuAddNextItem(GenMenu, 0, Menu2_SubMenu2_Alt_ChangeARRDraw, Menu2_SubMenu2_Alt_ChangeARREvents);
	  //подменю 3 к меню 2
	  MenuAddNextItem(GenMenu, 0, Menu2_SubMenu3_Alt_ChangeStepsCCRDraw, Menu2_SubMenu3_Alt_ChangeStepsCCREvents);
		if ( GenConfig.signalType != signalSquare )
		{
			//подменю 4 к меню 2
	    MenuAddNextItem(GenMenu, 0, Menu2_SubMenu3_ChangeSignalPowerDraw, Menu2_SubMenu3_ChangeSignalPowerEvents);	
			//подменю 5 к меню 2
	    MenuAddNextItem(GenMenu, 0, Menu2_SubMenu4_ChangeSignalCenterDraw, Menu2_SubMenu4_ChangeSignalCenterEvents);			
		}
	}
}

void FreeDataWhenTransition(void)
{
	DestroyAnimVarNumber(&valMain);
	DestroyAnimVarNumber(&valIncSize);
	incSize = 1;
	cursorNumN = 1;
	bufferVal = 0;
	bufferVarK = 0;
  menu_iteration_int1 = 0;
  menu_iteration_int2 = 0;
  menu_iteration_uint1 = 0;
  menu_iteration_uint2 = 0;
	menu_int32 = 0;
}

void LedUpdate(void)
{
	//светодиоды для индикации
	if ( GenConfig.isImmediateUpdate ) LED_BLUE_ON; else LED_BLUE_OFF;
	if ( GenConfig.ShowFreqType == ShowFreqTypeDirectControl ) LED_GREEN_ON; else LED_GREEN_OFF;
}

void Buttons_1_2_scan(SYS_EVENTS_DATA genEvents)
{
	if ( genEvents & EVENT_BUTTON1_CLICK )
	{
		//read from flash
		memset(&GenConfig, 0, sizeof(GenConfig));
		FlashReadData((uint8_t*)&GenConfig, sizeof(GenConfig));
	  //проверка наличия сохраненных данных
	  if ( GenConfig.signalType == 0xFF )
	  {
		  //данные по-умолчанию
		  GenConfig.isImmediateUpdate = defaultUpdateType;
		  GenConfig.ShowFreqType = defaultShowFreqType;
		  GenConfig.languageId = defaultLocLanguage;
		  GenConfig.freqPWM= defaultFreqPWM;
		  GenConfig.freqSignal = defaultFreqSignal;
		  GenConfig.powerK = defaultPowerK;
		  GenConfig.centerK = defaultSignalCenter;
		  GenConfig.pwmMinPulseLengthInNS = defaultTransistorsMinTime;
		  GenConfig.pwmDeadTimeInNS = defaultTransistorsDeadTime;
		  GenConfig.signalType = defaultSignalType;
		  GenConfig.timerPrescaler = defaultTimerPrescaler;
		  GenConfig.timerARR = defaultTimerARR;
		  GenConfig.timerStepsCCR = defaultTimerStepsCCR;
	  };
	}
	
	if ( genEvents & EVENT_BUTTON2_CLICK )
	{
	  if ( GenConfig.ShowFreqType != ShowFreqTypeDirectControl )
	  {
		  GenConfig.timerPrescaler = GenGetPrescalerValue(GenConfig.freqPWM, GenConfig.signalType);
	    GenConfig.timerARR = GenGetARRValueFromFreq(GenConfig.timerPrescaler, GenConfig.freqPWM, GenConfig.signalType);
	    GenConfig.timerStepsCCR = GenGetStepsCCRValueFromFreq(GenConfig.timerPrescaler, GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.timerARR,
                                                            (double)GenConfig.powerK / 10000, GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS,
		                                                        GenConfig.signalType);
	  }
		GenUpdateSignal(GenConfig.timerPrescaler, GenConfig.timerARR, GenConfig.timerStepsCCR, (double)GenConfig.powerK / 10000,
		                (double)GenConfig.centerK / 100, GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		LastMenuItemID = GenMenu->MenuCurrentItem->MenuItemId;
		GenMenu->MenuTransitionTimeInMS = 0;
		MenuGoToItemId(GenMenu, MenuOKItemID);
	}	
}

void _clearDisplayBuffers(void)
{
	uint8_t i;
	for ( i = 0; i < 16; i++ )
	{
		Line1Buffer[i] = ' ';
		Line2Buffer[i] = ' ';
	}
	Line1Buffer[16] = 0;
  Line2Buffer[16] = 0;
}

void _clearDisplayBuffer1(void)
{
	uint8_t i;
	for ( i = 0; i < 16; i++ )
	{
		Line1Buffer[i] = ' ';
	}
	Line1Buffer[16] = 0;
}

void _clearDisplayBuffer2(void)
{
	uint8_t i;
	for ( i = 0; i < 16; i++ )
	{
		Line2Buffer[i] = ' ';
	}
	Line2Buffer[16] = 0;
}

void _clearDisplayNewBuffers(void)
{
	uint8_t i;
	for ( i = 0; i < 16; i++ )
	{
		Line1BufferNew[i] = ' ';
		Line2BufferNew[i] = ' ';
	}
	Line1BufferNew[16] = 0;
	Line2BufferNew[16] = 0;
}

void _copyStringToBufferLtoR(char *_buffer, char *_string, uint8_t _posL)
{
	uint8_t i = 0;
	while ( ( ( _posL + i ) < 17 ) && ( _string[i] ) )
	{
		_buffer[_posL + i - 1] = _string[i];
		i++;
	}
	_buffer[16] = 0;
}

void _copyStringToBufferRtoL(char *_buffer, char *_string, uint8_t _strLen, uint8_t _posR)
{
	uint8_t i = 0;
	if ( !_strLen ) return;	
	while ( ( ( _posR - i ) < 17 ) && ( ( _posR - i ) > 0 ) && ( (_strLen - i) > 0 ) && ( _string[i] ) )
	{
		_buffer[_posR - i - 1] = _string[_strLen - i - 1];
		i++;
	}
	_buffer[16] = 0;
}

uint8_t GetCenterPos(char *_string)
{
	return ( ( 16 - strlen(_string) ) / 2 + 1);
}

void GetFormattedDouble(const double _x, uint8_t _length, char *_buffer, uint8_t _bufferLength)
{
	uint8_t _a = 0;
	if ( !_length ) return;
	if ( !_buffer ) return;
	if ( _length > _bufferLength ) return;
	snprintf(_buffer, _bufferLength, "%d", (int32_t)_x);
	_a = strlen(_buffer);
	if ( _a > ( _length - 1 ) )
	{
		snprintf(_buffer, _bufferLength, "*");
		return;
	}
	_a = _length - _a - 1;
	snprintf(_buffer, _bufferLength, "%*.*lf", _length, _a, _x);
}

void MenuTimerUpdate(void)
{
	if ( GenConfig.ShowFreqType != ShowFreqTypeDirectControl )
	{
		GenConfig.timerPrescaler = GenGetPrescalerValue(GenConfig.freqPWM, GenConfig.signalType);
	  GenConfig.timerARR = GenGetARRValueFromFreq(GenConfig.timerPrescaler, GenConfig.freqPWM, GenConfig.signalType);
	  GenConfig.timerStepsCCR = GenGetStepsCCRValueFromFreq(GenConfig.timerPrescaler, GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.timerARR,
                                                          (double)GenConfig.powerK / 10000, GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS,
		                                                      GenConfig.signalType);
	}
	if ( GenConfig.ShowFreqType == ShowFreqTypeActualFreq )
	{
	  GenConfig.freqPWM = GenGetPWMFreqValueFromTimer(GenConfig.timerPrescaler, GenConfig.timerARR, GenConfig.signalType);
		GenConfig.freqSignal = GenGetSignalFreqValueFromTimer(GenConfig.timerPrescaler, GenConfig.timerARR, GenConfig.timerStepsCCR, GenConfig.signalType);
	}
	if ( GenConfig.isImmediateUpdate )
	{
	  GenUpdateSignal(GenConfig.timerPrescaler, GenConfig.timerARR, GenConfig.timerStepsCCR, (double)GenConfig.powerK / 10000,
		                (double)GenConfig.centerK / 100, GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
	}	
}

void MenuResetSignalVal(void)
{
	GenConfig.freqPWM = defaultFreqPWM;
	GenConfig.freqSignal = defaultFreqSignal;
	GenConfig.powerK = defaultPowerK;
	GenConfig.centerK = defaultSignalCenter;
	GenConfig.timerPrescaler = GenGetPrescalerValue(GenConfig.freqPWM, GenConfig.signalType);
	GenConfig.timerARR = GenGetARRValueFromFreq(GenConfig.timerPrescaler, GenConfig.freqPWM, GenConfig.signalType);
	GenConfig.timerStepsCCR = GenGetStepsCCRValueFromFreq(GenConfig.timerPrescaler, GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.timerARR,
                                                        (double)GenConfig.powerK / 10000, GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS,
		                                                    GenConfig.signalType);
}

//*****************************************************************************************************************************//
//*****************************************************************************************************************************//
// main
//*****************************************************************************************************************************//
//*****************************************************************************************************************************//

void MenuTransitionDraw(const uint16_t frameNum)
{	
	uint8_t i = 0;
	
	if ( (frameNum % 2) != 0 ) return;
	
	switch ( menuTransDirection )
	{
		case 1: //вправо(-)
			for ( i = 15; i > 0; i-- )
	    {
				Line1Buffer[i] = Line1Buffer[i-1];
		    Line2Buffer[i] = Line2Buffer[i-1];
	    };
	    Line1Buffer[0] = Line1BufferNew[15];
	    Line2Buffer[0] = Line2BufferNew[15];
	    for ( i = 15; i > 0; i-- )
			{
				Line1BufferNew[i] = Line1BufferNew[i-1];
		    Line2BufferNew[i] = Line2BufferNew[i-1];
	    };
	    Line1BufferNew[0] = ' ';
	    Line2BufferNew[0] = ' ';
		break;
			
		case 2: //влево(+)
			for ( i = 0; i < 15; i++ )
		  {
				Line1Buffer[i] = Line1Buffer[i+1];
				Line2Buffer[i] = Line2Buffer[i+1];
	    };
			Line1Buffer[15] = Line1BufferNew[0];
			Line2Buffer[15] = Line2BufferNew[0];
			for ( i = 0; i < 15; i++ )
			{
				Line1BufferNew[i] = Line1BufferNew[i+1];
				Line2BufferNew[i] = Line2BufferNew[i+1];
	    };
			Line1BufferNew[15] = ' ';
			Line2BufferNew[15] = ' ';
		break;
	};
	
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
}

//Menu HI
uint8_t MenuHiDraw(const uint8_t frameNum)
{
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], locLanguageData[0], GetCenterPos(locLanguageData[0]));	
	//2 строка
	_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[1], 1);
	_copyStringToBufferLtoR(&Line2Buffer[0], buildID, 7);
	_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[1], 11);
	
	//выходим после 1с
	if ( frameNum >= GenMenu->MenuTargetDrawFPS )
	{
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
	};
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

//Menu OK
uint8_t MenuOKDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
	  //1 строка
	  _copyStringToBufferLtoR(&Line1BufferNew[0], locLanguageData[2], GetCenterPos(locLanguageData[2]));
		return RESULT_OK;
	};
	
	if ( frameNum > GenMenu->MenuTargetDrawFPS / 2 )
	{
		MenuGoToItemId(GenMenu, LastMenuItemID);
		LastMenuItemID = 0;
		GenMenu->MenuTransitionTimeInMS = MENU_TRANS_TIME;		
	};
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], locLanguageData[2], GetCenterPos(locLanguageData[2]));
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

//Menu1 - Start Menu
uint8_t Menu1_StartMenuDraw(const uint8_t frameNum)
{	
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		MenuTickerStrDraw(&Line1BufferNew[0], locLanguageData[3], 1, 8, &menu_iteration_uint1);
	  snprintf(displayString, 17, "%d", (uint32_t)GenGetPWMFreqValueFromTimer(GenConfig.timerPrescaler, GenConfig.timerARR, GenConfig.signalType));
	  _copyStringToBufferRtoL(&Line1BufferNew[0], displayString, strlen(displayString), 16);
		
		MenuTickerStrDraw(&Line2BufferNew[0], locLanguageData[4], 1, 8, &menu_iteration_uint2);
	  snprintf(displayString, 17, "%d", (uint32_t)GenGetSignalFreqValueFromTimer(GenConfig.timerPrescaler, GenConfig.timerARR, GenConfig.timerStepsCCR, GenConfig.signalType));
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 16);
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	if ( !( frameNum % 25 ) )
	{
		_clearDisplayBuffers();
	  MenuTickerStrDraw(&Line1Buffer[0], locLanguageData[3], 1, 8, &menu_iteration_uint1);
	  snprintf(displayString, 17, "%d", (uint32_t)GenGetPWMFreqValueFromTimer(GenConfig.timerPrescaler, GenConfig.timerARR, GenConfig.signalType));
	  _copyStringToBufferRtoL(&Line1Buffer[0], displayString, strlen(displayString), 16);
	
	  MenuTickerStrDraw(&Line2Buffer[0], locLanguageData[4], 1, 8, &menu_iteration_uint2);
	  snprintf(displayString, 17, "%d", (uint32_t)GenGetSignalFreqValueFromTimer(GenConfig.timerPrescaler, GenConfig.timerARR, GenConfig.timerStepsCCR, GenConfig.signalType));
	  _copyStringToBufferRtoL(&Line2Buffer[0], displayString, strlen(displayString), 16);
	};
		
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

uint8_t Menu1_StartMenuEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода		
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
  };
	
	Buttons_1_2_scan(genEvents);
	return RESULT_OK;
}

//Menu2 - Main Menu
uint8_t Menu2_MainMenuDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
	  //1 строка
		MenuFloatingStrDraw(&Line1BufferNew[0], locLanguageData[5], 1, 16, &menu_iteration_int1);	
	  //2 строка
		MenuTickerStrDraw(&Line2BufferNew[0], locLanguageData[6], 1, 16, &menu_iteration_uint2);
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//очистка
	//_clearDisplayBuffers(); без очистки
	if ( !( frameNum % 25 ) )
	{
		//1 строка
		MenuFloatingStrDraw(&Line1Buffer[0], locLanguageData[5], 1, 16, &menu_iteration_int1);	
	  //2 строка
		MenuTickerStrDraw(&Line2Buffer[0], locLanguageData[6], 1, 16, &menu_iteration_uint2);
	};
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

uint8_t Menu2_MainMenuEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{	
	//нажатие 3 или 4
	if ( ( genEvents & EVENT_BUTTON3_CLICK ) || ( genEvents & EVENT_BUTTON4_CLICK ) )
  {
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода		
		menuTransDirection = 2; //влево(+)
		MenuGoToChildItem(GenMenu);
		
		//выходим
		return RESULT_OK;
  };
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода		
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
  };
	
	Buttons_1_2_scan(genEvents);
	return RESULT_OK;
}

//Menu3 - Extra Menu
uint8_t Menu3_ExtraMenuDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
	  //1 строка
		MenuFloatingStrDraw(&Line1BufferNew[0], locLanguageData[7], 1, 16, &menu_iteration_int1);	
	  //2 строка
		MenuTickerStrDraw(&Line2BufferNew[0], locLanguageData[8], 1, 16, &menu_iteration_uint2);
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//очистка
	//_clearDisplayBuffers(); без очистки
	if ( !( frameNum % 25 ) )
	{
		//1 строка
		MenuFloatingStrDraw(&Line1Buffer[0], locLanguageData[7], 1, 16, &menu_iteration_int1);	
	  //2 строка
		MenuTickerStrDraw(&Line2Buffer[0], locLanguageData[8], 1, 16, &menu_iteration_uint2);
	};
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}
	
uint8_t Menu3_ExtraMenuEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{	
	//нажатие 3 или 4
	if ( ( genEvents & EVENT_BUTTON3_CLICK ) || ( genEvents & EVENT_BUTTON4_CLICK ) )
  {
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода		
		menuTransDirection = 2; //влево(+)
		MenuGoToChildItem(GenMenu);
		
		//выходим
		return RESULT_OK;
  };
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода		
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
  };
	
	Buttons_1_2_scan(genEvents);
	return RESULT_OK;
}

//Menu4 - Save Menu
uint8_t Menu4_SaveMenuDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
	  //1 строка
		MenuFloatingStrDraw(&Line1BufferNew[0], locLanguageData[9], 1, 16, &menu_iteration_int1);	
	  //2 строка
		MenuTickerStrDraw(&Line2BufferNew[0], locLanguageData[10], 1, 16, &menu_iteration_uint1);
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//очистка
	//_clearDisplayBuffers(); без очистки
	if ( !( frameNum % 25 ) )
	{
		//1 строка
		MenuFloatingStrDraw(&Line1Buffer[0], locLanguageData[9], 1, 16, &menu_iteration_int1);	
	  //2 строка
		MenuTickerStrDraw(&Line2Buffer[0], locLanguageData[10], 1, 16, &menu_iteration_uint1);
	};	
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

uint8_t Menu4_SaveMenuEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//нажатие 3 или 4
	if ( ( genEvents & EVENT_BUTTON3_CLICK ) || ( genEvents & EVENT_BUTTON4_CLICK ) )
	{
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода
		menuTransDirection = 2; //влево(+)
		MenuGoToChildItem(GenMenu);		
	};
	
	Buttons_1_2_scan(genEvents);
	return RESULT_OK;
}

//Menu2 - SubMenu1 - Change PWM Freq
uint8_t Menu2_SubMenu1_ChangePWMFreqDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		_copyStringToBufferLtoR(&Line1BufferNew[0], locLanguageData[11], 1);
		menu_int32 = (int32_t)GenConfig.freqPWM;
		snprintf(displayString, 17, "%d", menu_int32);
	  _copyStringToBufferRtoL(&Line1BufferNew[0], displayString, strlen(displayString), 16);
    //2 строка
	  _copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[12], 1);
	  snprintf(displayString, 17, "%d", incSize);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 16);
		return RESULT_OK;
	};
	
	//проверка полей ввода, создание при необходимости
	if ( ( !valMain ) && ( !valIncSize ) )
	{
		incSize = 1;
		valMain = CreateAnimVarNumber(&Display, 1, 16, &menu_int32, 0, &incSize);
		valIncSize = CreateAnimVarNumber(&Display, 2, 16, &incSize, 0, 0);	
    AnimVarNumberEnable(valMain, 1);	
    cursorNumN = 1;		
	}
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], locLanguageData[11], 1);	
	//2 строка
	_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[12], 1);
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	
	//выводим числа
	AnimVarNumberDraw(valMain);
	AnimVarNumberDraw(valIncSize);
	switch ( cursorNumN )
	{
		case 1:			
		  AnimVarNumberDrawCursor(valMain);
		break;
		
		case 2:			
		  AnimVarNumberDrawCursor(valIncSize);
		break;
	}
	
	return RESULT_OK;
}

uint8_t Menu2_SubMenu1_ChangePWMFreqEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	//событие при удерживании нажатой кнопки валкодера и поворота по ч стрелке
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CW ) )
	{
		AnimVarNumberCursorMoveRight(valMain);
		AnimVarNumberCursorMoveRight(valIncSize);
		return RESULT_OK;
	};
	
	//событие при удерживании нажатой кнопки валкодера и поворота против ч стрелки
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CCW ) )
	{
		AnimVarNumberCursorMoveLeft(valMain);
		AnimVarNumberCursorMoveLeft(valIncSize);
		return RESULT_OK;
	};
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToParentItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		switch ( cursorNumN )
		{
			case 1:
				bufferVal = menu_int32;
				AnimVarNumberDec(valMain);
			  if ( menu_int32 <= 0 )
				{
					menu_int32 = bufferVal;
					GenConfig.freqPWM = bufferVal;
				} else
				{
					if ( GenCheckFrequencyAvailable(menu_int32, GenConfig.freqSignal, GenConfig.signalType) )
					{
						if ( GenConfig.ShowFreqType == ShowFreqTypeNormal )
						{
							GenConfig.freqPWM = menu_int32;
						  MenuTimerUpdate();
						}
						if ( GenConfig.ShowFreqType == ShowFreqTypeActualFreq )
						{
							bufferVarK = (int32_t)GetRealAvailablePWMFreq(GenConfig.timerPrescaler, menu_int32, GenConfig.freqSignal, GenConfig.signalType);
							if ( ( bufferVarK > 0 ) && ( bufferVarK == bufferVal ) )
							{
								bufferVarK = (int32_t)GetPrevAvailablePWMFreq(GenConfig.timerPrescaler, bufferVal, GenConfig.freqSignal, GenConfig.signalType);
							}
							if ( ( bufferVarK > 0 ) && ( bufferVarK != bufferVal ) )
							{
								menu_int32 = bufferVarK;
								GenConfig.freqPWM = bufferVarK;
								MenuTimerUpdate();
							} else
					    {
						    menu_int32 = bufferVal;
                GenConfig.freqPWM = bufferVal;						
					    }
						}
					} else
					{
						menu_int32 = bufferVal;
            GenConfig.freqPWM = bufferVal;						
					}
			  }
		  break;
			case 2:
				AnimVarNumberDec(valIncSize);
				if ( incSize < DeltaValMinValue )
					incSize = DeltaValMinValue;
		  break;
	  };
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		switch ( cursorNumN )
		{
			case 1:
				bufferVal = menu_int32;
				AnimVarNumberInc(valMain);
			  if ( menu_int32 <= 0 )
				{
					menu_int32 = bufferVal;
					GenConfig.freqPWM = bufferVal;
				} else
				{
					if ( GenCheckFrequencyAvailable(menu_int32, GenConfig.freqSignal, GenConfig.signalType) )
					{
						if ( GenConfig.ShowFreqType == ShowFreqTypeNormal )
						{
							GenConfig.freqPWM = menu_int32;
						  MenuTimerUpdate();
						}
						if ( GenConfig.ShowFreqType == ShowFreqTypeActualFreq )
						{
							bufferVarK = (int32_t)GetRealAvailablePWMFreq(GenConfig.timerPrescaler, menu_int32, GenConfig.freqSignal, GenConfig.signalType);
							if ( ( bufferVarK > 0 ) && ( bufferVarK == bufferVal ) )
							{
								bufferVarK = (int32_t)GetNextAvailablePWMFreq(GenConfig.timerPrescaler, bufferVal, GenConfig.freqSignal, GenConfig.signalType);
							}
							if ( ( bufferVarK > 0 ) && ( bufferVarK != bufferVal ) )
							{
								menu_int32 = bufferVarK;
								GenConfig.freqPWM = bufferVarK;
								MenuTimerUpdate();
							} else
					    {
						    menu_int32 = bufferVal;
                GenConfig.freqPWM = bufferVal;						
					    }
						}
					} else
					{
						menu_int32 = bufferVal;
            GenConfig.freqPWM = bufferVal;						
					}
			  }
		  break;
			case 2:
				AnimVarNumberInc(valIncSize);
				if ( incSize > DeltaValMaxValue )
					incSize = DeltaValMaxValue;
		  break;
	  };
	};
	
	//событие при нажатии кнопки 3
	if ( genEvents & EVENT_BUTTON3_CLICK )
	{
		cursorNumN = 1;
		AnimVarNumberEnable(valMain, 1);
		AnimVarNumberEnable(valIncSize, 0);
	};
	
	//событие при нажатии кнопки 4
	if ( genEvents & EVENT_BUTTON4_CLICK )
	{
		cursorNumN = 2;
		AnimVarNumberEnable(valMain, 0);
		AnimVarNumberEnable(valIncSize, 1);
	};
	
	//кнопки 1 - данные по-умолчанию, 2 - ОК
	Buttons_1_2_scan(genEvents);	
	if ( genEvents & EVENT_BUTTON1_CLICK )
	{
		menu_int32 = (int32_t)GenConfig.freqPWM;
		incSize = DeltaValMinValue;
	}
	
	return RESULT_OK;
}

//Menu2 - SubMenu2 - Change Signal Freq
uint8_t Menu2_SubMenu2_ChangeSignalFreqDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		_copyStringToBufferLtoR(&Line1BufferNew[0], locLanguageData[13], 1);
		menu_int32 = (int32_t)GenConfig.freqSignal;
		snprintf(displayString, 17, "%d", menu_int32);
	  _copyStringToBufferRtoL(&Line1BufferNew[0], displayString, strlen(displayString), 16);
    //2 строка
	  _copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[12], 1);
	  snprintf(displayString, 17, "%d", incSize);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 16);
		return RESULT_OK;
	};
	
	//проверка полей ввода, создание при необходимости
	if ( ( !valMain ) && ( !valIncSize ) )
	{
		incSize = 1;
		valMain = CreateAnimVarNumber(&Display, 1, 16, &menu_int32, 0, &incSize);
		valIncSize = CreateAnimVarNumber(&Display, 2, 16, &incSize, 0, 0);	
    AnimVarNumberEnable(valMain, 1);	
    cursorNumN = 1;		
	}
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], locLanguageData[13], 1);	
	//2 строка
	_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[12], 1);
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	
	//выводим числа
	AnimVarNumberDraw(valMain);
	AnimVarNumberDraw(valIncSize);
	switch ( cursorNumN )
	{
		case 1:			
		  AnimVarNumberDrawCursor(valMain);
		break;
		
		case 2:			
		  AnimVarNumberDrawCursor(valIncSize);
		break;
	}
	
	return RESULT_OK;
}

uint8_t Menu2_SubMenu2_ChangeSignalFreqEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	//событие при удерживании нажатой кнопки валкодера и поворота по ч стрелке
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CW ) )
	{
		AnimVarNumberCursorMoveRight(valMain);
		AnimVarNumberCursorMoveRight(valIncSize);
		return RESULT_OK;
	};
	
	//событие при удерживании нажатой кнопки валкодера и поворота против ч стрелки
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CCW ) )
	{
		AnimVarNumberCursorMoveLeft(valMain);
		AnimVarNumberCursorMoveLeft(valIncSize);
		return RESULT_OK;
	};
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		switch ( cursorNumN )
		{
			case 1:
				bufferVal = menu_int32;
				AnimVarNumberDec(valMain);
			  if ( menu_int32 <= 0 )
				{
					menu_int32 = bufferVal;
					GenConfig.freqSignal = bufferVal;
				} else
				{
					if ( GenCheckFrequencyAvailable(GenConfig.freqPWM, menu_int32, GenConfig.signalType) )
					{
						if ( GenConfig.ShowFreqType == ShowFreqTypeNormal )
						{
							GenConfig.freqSignal = menu_int32;
						  MenuTimerUpdate();
						}
						if ( GenConfig.ShowFreqType == ShowFreqTypeActualFreq )
						{
							bufferVarK = (int32_t)GetRealAvailableSignalFreq(GenConfig.timerPrescaler, GenConfig.freqPWM, menu_int32, GenConfig.signalType);
							if ( ( bufferVarK > 0 ) && ( bufferVarK == bufferVal ) )
							{
								bufferVarK = (int32_t)GetPrevAvailableSignalFreq(GenConfig.timerPrescaler, GenConfig.freqPWM, bufferVal, GenConfig.signalType);
							}
							if ( ( bufferVarK > 0 ) && ( bufferVarK != bufferVal ) )
							{
								menu_int32 = bufferVarK;
								GenConfig.freqSignal = bufferVarK;
								MenuTimerUpdate();
							} else
							{
								menu_int32 = bufferVal;
								GenConfig.freqSignal = bufferVal;
							}
						}
					} else
					{
						menu_int32 = bufferVal;
            GenConfig.freqSignal = bufferVal;						
					}
			  }
		  break;
			case 2:
				AnimVarNumberDec(valIncSize);
				if ( incSize < DeltaValMinValue )
					incSize = DeltaValMinValue;
		  break;
	  };
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		switch ( cursorNumN )
		{
			case 1:
				bufferVal = menu_int32;
				AnimVarNumberInc(valMain);
			  if ( menu_int32 <= 0 )
				{
					menu_int32 = bufferVal;
					GenConfig.freqSignal = bufferVal;
				} else
				{
					if ( GenCheckFrequencyAvailable(GenConfig.freqPWM, menu_int32, GenConfig.signalType) )
					{
						if ( GenConfig.ShowFreqType == ShowFreqTypeNormal )
						{
							GenConfig.freqSignal = menu_int32;
						  MenuTimerUpdate();
						}
						if ( GenConfig.ShowFreqType == ShowFreqTypeActualFreq )
						{
							bufferVarK = (int32_t)GetRealAvailableSignalFreq(GenConfig.timerPrescaler, GenConfig.freqPWM, menu_int32, GenConfig.signalType);
							if ( ( bufferVarK > 0 ) && ( bufferVarK == bufferVal ) )
							{
								bufferVarK = (int32_t)GetNextAvailableSignalFreq(GenConfig.timerPrescaler, GenConfig.freqPWM, bufferVal, GenConfig.signalType);
							}
							if ( ( bufferVarK > 0 ) && ( bufferVarK != bufferVal ) )
							{
								menu_int32 = bufferVarK;
								GenConfig.freqSignal = bufferVarK;
								MenuTimerUpdate();
							} else
							{
								menu_int32 = bufferVal;
								GenConfig.freqSignal = bufferVal;
							}
						}
					} else
					{
						menu_int32 = bufferVal;
            GenConfig.freqSignal = bufferVal;						
					}
			  }
		  break;
			case 2:
				AnimVarNumberInc(valIncSize);
				if ( incSize > DeltaValMaxValue )
					incSize = DeltaValMaxValue;
		  break;
	  };
	};
	
	//событие при нажатии кнопки 3
	if ( genEvents & EVENT_BUTTON3_CLICK )
	{
		cursorNumN = 1;
		AnimVarNumberEnable(valMain, 1);
		AnimVarNumberEnable(valIncSize, 0);
	};
	
	//событие при нажатии кнопки 4
	if ( genEvents & EVENT_BUTTON4_CLICK )
	{
		cursorNumN = 2;
		AnimVarNumberEnable(valMain, 0);
		AnimVarNumberEnable(valIncSize, 1);
	};
	
	//кнопки 1 - данные по-умолчанию, 2 - ОК
	Buttons_1_2_scan(genEvents);	
	if ( genEvents & EVENT_BUTTON1_CLICK )
	{
		menu_int32 = (int32_t)GenConfig.freqSignal;
		incSize = DeltaValMinValue;
	}
	
	return RESULT_OK;
}

//Menu2 - SubMenu3 - Change Signal Power
uint8_t Menu2_SubMenu3_ChangeSignalPowerDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		MenuFloatingStrDraw(&Line1BufferNew[0], locLanguageData[14], 1, 16, &menu_iteration_int1);
    //2 строка
	  snprintf(displayString, 17, "%.2lf", (double)GenConfig.powerK / 100);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 12);
		return RESULT_OK;
	};
	
	//проверка полей ввода, создание при необходимости
	if ( !valMain )
	{
		valMain = CreateAnimVarNumber(&Display, 2, 12, &GenConfig.powerK, 2, 0);
    AnimVarNumberEnable(valMain, 1);		
	}

	//1 строка
	if ( !( frameNum % 25 ) )
	{
		_clearDisplayBuffer1();
		MenuFloatingStrDraw(&Line1Buffer[0], locLanguageData[14], 1, 16, &menu_iteration_int1);	
	};
	
	//2 строка
	_clearDisplayBuffer2();
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	
	//выводим числа
	AnimVarNumberDraw(valMain);		
	AnimVarNumberDrawCursor(valMain);
	
	return RESULT_OK;
}

uint8_t Menu2_SubMenu3_ChangeSignalPowerEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	//событие при удерживании нажатой кнопки валкодера и поворота по ч стрелке
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CW ) )
	{
		AnimVarNumberCursorMoveRight(valMain);
		return RESULT_OK;
	};
	
	//событие при удерживании нажатой кнопки валкодера и поворота против ч стрелки
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CCW ) )
	{
		AnimVarNumberCursorMoveLeft(valMain);
		return RESULT_OK;
	};
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		if ( ( GenConfig.ShowFreqType == ShowFreqTypeNormal ) && ( GenConfig.signalType == signalSquare ) )
		{
			MenuGoToParentItem(GenMenu);
		} else
		{
		  MenuGoToNextItem(GenMenu);
		}
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		AnimVarNumberDec(valMain);
		if ( (double)GenConfig.powerK / 10000 < MIN_POWER_K )
			GenConfig.powerK = MIN_POWER_K * 10000;
    MenuTimerUpdate();
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		AnimVarNumberInc(valMain);
		if ( (double)GenConfig.powerK / 10000 > MAX_POWER_K )
			GenConfig.powerK = MAX_POWER_K * 10000;
    MenuTimerUpdate();
	};
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}

//Menu2 - SubMenu4 - Change Signal Center
uint8_t Menu2_SubMenu4_ChangeSignalCenterDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		MenuFloatingStrDraw(&Line1BufferNew[0], locLanguageData[15], 1, 16, &menu_iteration_int1);
    //2 строка
	  snprintf(displayString, 17, "%.2lf", (double)GenConfig.centerK / 100);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 12);
		return RESULT_OK;
	};
	
	//проверка полей ввода, создание при необходимости
	if ( !valMain )
	{
		valMain = CreateAnimVarNumber(&Display, 2, 12, &GenConfig.centerK, 2, 0);
    AnimVarNumberEnable(valMain, 1);		
	}

	//1 строка
	if ( !( frameNum % 25 ) )
	{
		_clearDisplayBuffer1();
		MenuFloatingStrDraw(&Line1Buffer[0], locLanguageData[15], 1, 16, &menu_iteration_int1);	
	};
	
	//2 строка
	_clearDisplayBuffer2();
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	
	//выводим числа
	AnimVarNumberDraw(valMain);		
	AnimVarNumberDrawCursor(valMain);
	
	return RESULT_OK;
}

uint8_t Menu2_SubMenu4_ChangeSignalCenterEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	//событие при удерживании нажатой кнопки валкодера и поворота по ч стрелке
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CW ) )
	{
		AnimVarNumberCursorMoveRight(valMain);
		return RESULT_OK;
	};
	
	//событие при удерживании нажатой кнопки валкодера и поворота против ч стрелки
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CCW ) )
	{
		AnimVarNumberCursorMoveLeft(valMain);
		return RESULT_OK;
	};
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		MenuGoToParentItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		AnimVarNumberDec(valMain);
		if ( GenConfig.centerK < MIN_CENTER_K )
			GenConfig.centerK = MIN_CENTER_K;
    MenuTimerUpdate();
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		AnimVarNumberInc(valMain);
		if ( GenConfig.centerK > MAX_CENTER_K )
			GenConfig.centerK = MAX_CENTER_K;
    MenuTimerUpdate();
	};
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}

//Menu2 - SubMenu1 - alt - Change Prescaler
uint8_t Menu2_SubMenu1_Alt_ChangePrescalerDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		_copyStringToBufferLtoR(&Line1BufferNew[0], locLanguageData[16], 1);
		_copyStringToBufferRtoL(&Line1BufferNew[0], locLanguageData[17], strlen(locLanguageData[17]), 16);
    //2 строка
		menu_int32 = GenConfig.timerPrescaler + 1;
		snprintf(displayString, 17, "%d", menu_int32);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 5);		
		GetFormattedDouble((double)cpuFreq / menu_int32, 9, displayString, 17);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 16);
		return RESULT_OK;
	};
	
	//проверка полей ввода, создание при необходимости
	if ( !valMain )
	{
		valMain = CreateAnimVarNumber(&Display, 2, 5, &menu_int32, 0, 0);
    AnimVarNumberEnable(valMain, 1);		
	}
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], locLanguageData[16], 1);
	_copyStringToBufferRtoL(&Line1Buffer[0], locLanguageData[17], strlen(locLanguageData[17]), 16);
  //2 строка		
	GetFormattedDouble((double)cpuFreq / menu_int32, 9, displayString, 17);
	_copyStringToBufferRtoL(&Line2Buffer[0], displayString, strlen(displayString), 16);

	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	
	//выводим числа
	AnimVarNumberDraw(valMain);		
	AnimVarNumberDrawCursor(valMain);
	
	return RESULT_OK;
}

uint8_t Menu2_SubMenu1_Alt_ChangePrescalerEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	//событие при удерживании нажатой кнопки валкодера и поворота по ч стрелке
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CW ) )
	{
		AnimVarNumberCursorMoveRight(valMain);
		return RESULT_OK;
	};
	
	//событие при удерживании нажатой кнопки валкодера и поворота против ч стрелки
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CCW ) )
	{
		AnimVarNumberCursorMoveLeft(valMain);
		return RESULT_OK;
	};
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToParentItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		bufferVal = menu_int32;
		AnimVarNumberDec(valMain);
		bufferVarK = GenCheckSignalConfig(menu_int32 - 1, GenConfig.timerARR, GenConfig.timerStepsCCR,
		                                  (double)GenConfig.powerK / 10000, GenConfig.centerK, GenConfig.pwmMinPulseLengthInNS,
		                                  GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		if ( !bufferVarK )
		{
			menu_int32 = bufferVal;
		}
		GenConfig.timerPrescaler = menu_int32 - 1;
    MenuTimerUpdate();
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		bufferVal = menu_int32;
		AnimVarNumberInc(valMain);
		bufferVarK = GenCheckSignalConfig(menu_int32 - 1, GenConfig.timerARR, GenConfig.timerStepsCCR,
		                                  (double)GenConfig.powerK / 10000, GenConfig.centerK, GenConfig.pwmMinPulseLengthInNS,
		                                  GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		if ( !bufferVarK )
		{
			menu_int32 = bufferVal;
		}
		GenConfig.timerPrescaler = menu_int32 - 1;
    MenuTimerUpdate();
	};
	
	//кнопки 1 - данные по-умолчанию, 2 - ОК
	Buttons_1_2_scan(genEvents);	
	if ( genEvents & EVENT_BUTTON1_CLICK )
	{
    menu_int32 = GenConfig.timerPrescaler + 1;
	}
	
	return RESULT_OK;
}

//Menu2 - SubMenu2 - alt - Change ARR
uint8_t Menu2_SubMenu2_Alt_ChangeARRDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		_copyStringToBufferLtoR(&Line1BufferNew[0], locLanguageData[18], 1);
		_copyStringToBufferRtoL(&Line1BufferNew[0], locLanguageData[11], strlen(locLanguageData[11]), 16);
    //2 строка
		menu_int32 = GenConfig.timerARR;
		snprintf(displayString, 17, "%d", menu_int32);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 5);		
		GetFormattedDouble(GenGetPWMFreqValueFromTimer(GenConfig.timerPrescaler, GenConfig.timerARR, GenConfig.signalType), 9, displayString, 17);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 16);
		return RESULT_OK;
	};
	
	//проверка полей ввода, создание при необходимости
	if ( !valMain )
	{
		valMain = CreateAnimVarNumber(&Display, 2, 5, &menu_int32, 0, 0);
    AnimVarNumberEnable(valMain, 1);		
	}
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], locLanguageData[18], 1);
	_copyStringToBufferRtoL(&Line1Buffer[0], locLanguageData[11], strlen(locLanguageData[11]), 16);
  //2 строка		
	GetFormattedDouble(GenGetPWMFreqValueFromTimer(GenConfig.timerPrescaler, GenConfig.timerARR, GenConfig.signalType), 9, displayString, 17);
	_copyStringToBufferRtoL(&Line2Buffer[0], displayString, strlen(displayString), 16);

	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	
	//выводим числа
	AnimVarNumberDraw(valMain);		
	AnimVarNumberDrawCursor(valMain);
	
	return RESULT_OK;
}

uint8_t Menu2_SubMenu2_Alt_ChangeARREvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	//событие при удерживании нажатой кнопки валкодера и поворота по ч стрелке
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CW ) )
	{
		AnimVarNumberCursorMoveRight(valMain);
		return RESULT_OK;
	};
	
	//событие при удерживании нажатой кнопки валкодера и поворота против ч стрелки
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CCW ) )
	{
		AnimVarNumberCursorMoveLeft(valMain);
		return RESULT_OK;
	};
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		bufferVal = menu_int32;
		AnimVarNumberDec(valMain);
		bufferVarK = GenCheckSignalConfig(GenConfig.timerPrescaler, menu_int32, GenConfig.timerStepsCCR,
		                                  (double)GenConfig.powerK / 10000, GenConfig.centerK, GenConfig.pwmMinPulseLengthInNS,
		                                  GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		if ( !bufferVarK )
		{
			menu_int32 = bufferVal;
		}
		GenConfig.timerARR = menu_int32;
    MenuTimerUpdate();
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		bufferVal = menu_int32;
		AnimVarNumberInc(valMain);
		bufferVarK = GenCheckSignalConfig(GenConfig.timerPrescaler, menu_int32, GenConfig.timerStepsCCR,
		                                  (double)GenConfig.powerK / 10000, GenConfig.centerK, GenConfig.pwmMinPulseLengthInNS,
		                                  GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		if ( !bufferVarK )
		{
			menu_int32 = bufferVal;
		}
		GenConfig.timerARR = menu_int32;
    MenuTimerUpdate();
	};
	
	//кнопки 1 - данные по-умолчанию, 2 - ОК
	Buttons_1_2_scan(genEvents);	
	if ( genEvents & EVENT_BUTTON1_CLICK )
	{
    menu_int32 = GenConfig.timerARR;
	}
	
	return RESULT_OK;
}

//Menu2 - SubMenu3 - alt - Change Signal Steps / CCR
uint8_t Menu2_SubMenu3_Alt_ChangeStepsCCRDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		_copyStringToBufferLtoR(&Line1BufferNew[0], locLanguageData[19], 1);
		_copyStringToBufferRtoL(&Line1BufferNew[0], locLanguageData[13], strlen(locLanguageData[13]), 16);
    //2 строка
		menu_int32 = GenConfig.timerStepsCCR;
		snprintf(displayString, 17, "%d", menu_int32);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 5);		
		GetFormattedDouble(GenGetSignalFreqValueFromTimer(GenConfig.timerPrescaler, GenConfig.timerARR, GenConfig.timerStepsCCR, GenConfig.signalType), 9, displayString, 17);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 16);
		return RESULT_OK;
	};
	
	//проверка полей ввода, создание при необходимости
	if ( !valMain )
	{
		valMain = CreateAnimVarNumber(&Display, 2, 5, &menu_int32, 0, 0);
    AnimVarNumberEnable(valMain, 1);		
	}
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], locLanguageData[19], 1);
	_copyStringToBufferRtoL(&Line1Buffer[0], locLanguageData[13], strlen(locLanguageData[13]), 16);
  //2 строка		
  GetFormattedDouble(GenGetSignalFreqValueFromTimer(GenConfig.timerPrescaler, GenConfig.timerARR, GenConfig.timerStepsCCR, GenConfig.signalType), 9, displayString, 17);
	_copyStringToBufferRtoL(&Line2Buffer[0], displayString, strlen(displayString), 16);

	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	
	//выводим числа
	AnimVarNumberDraw(valMain);		
	AnimVarNumberDrawCursor(valMain);
	
	return RESULT_OK;
}

uint8_t Menu2_SubMenu3_Alt_ChangeStepsCCREvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	//событие при удерживании нажатой кнопки валкодера и поворота по ч стрелке
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CW ) )
	{
		AnimVarNumberCursorMoveRight(valMain);
		return RESULT_OK;
	};
	
	//событие при удерживании нажатой кнопки валкодера и поворота против ч стрелки
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CCW ) )
	{
		AnimVarNumberCursorMoveLeft(valMain);
		return RESULT_OK;
	};
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		if ( GenConfig.signalType == signalSquare )
		{
		  MenuGoToParentItem(GenMenu);
		} else
		{
		  MenuGoToNextItem(GenMenu);
		}
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		bufferVal = menu_int32;
		AnimVarNumberDec(valMain);
		bufferVarK = GenCheckSignalConfig(GenConfig.timerPrescaler, GenConfig.timerARR, menu_int32,
		                                  (double)GenConfig.powerK / 10000, GenConfig.centerK, GenConfig.pwmMinPulseLengthInNS,
		                                  GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		if ( !bufferVarK )
		{
			menu_int32 = bufferVal;
		}
		GenConfig.timerStepsCCR = menu_int32;
    MenuTimerUpdate();
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		bufferVal = menu_int32;
		AnimVarNumberInc(valMain);
		bufferVarK = GenCheckSignalConfig(GenConfig.timerPrescaler, GenConfig.timerARR, menu_int32,
		                                  (double)GenConfig.powerK / 10000, GenConfig.centerK, GenConfig.pwmMinPulseLengthInNS,
		                                  GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		if ( !bufferVarK )
		{
			menu_int32 = bufferVal;
		}
		GenConfig.timerStepsCCR = menu_int32;
    MenuTimerUpdate();
	};
	
	//кнопки 1 - данные по-умолчанию, 2 - ОК
	Buttons_1_2_scan(genEvents);	
	if ( genEvents & EVENT_BUTTON1_CLICK )
	{
    menu_int32 = GenConfig.timerStepsCCR;
	}
	
	return RESULT_OK;
}

//Menu3 - SubMenu1 - Change Dead Time
uint8_t Menu3_SubMenu1_ChangeDeadTimeDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		MenuFloatingStrDraw(&Line1BufferNew[0], locLanguageData[20], 1, 16, &menu_iteration_int1);
    //2 строка
		_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[21], 12);
	  snprintf(displayString, 17, "%d", GenConfig.pwmDeadTimeInNS);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 10);
		return RESULT_OK;
	};
	
	//проверка полей ввода, создание при необходимости
	if ( !valMain )
	{
		valMain = CreateAnimVarNumber(&Display, 2, 10, &GenConfig.pwmDeadTimeInNS, 0, 0);	
    AnimVarNumberEnable(valMain, 1);			
	}
	
	//1 строка
	if ( !( frameNum % 25 ) )
	{
		_clearDisplayBuffer1();
		MenuFloatingStrDraw(&Line1Buffer[0], locLanguageData[20], 1, 16, &menu_iteration_int1);	
	};
	
	//2 строка
	_clearDisplayBuffer2();
	_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[21], 12);
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	
	//выводим числа
	AnimVarNumberDraw(valMain);			
	AnimVarNumberDrawCursor(valMain);
	
	return RESULT_OK;
}

uint8_t Menu3_SubMenu1_ChangeDeadTimeEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	//событие при удерживании нажатой кнопки валкодера и поворота по ч стрелке
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CW ) )
	{
		AnimVarNumberCursorMoveRight(valMain);
		return RESULT_OK;
	};
	
	//событие при удерживании нажатой кнопки валкодера и поворота против ч стрелки
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CCW ) )
	{
		AnimVarNumberCursorMoveLeft(valMain);
		return RESULT_OK;
	};
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToParentItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		AnimVarNumberDec(valMain);
		if ( GenConfig.pwmDeadTimeInNS < MIN_DEAD_TIME )
			GenConfig.pwmDeadTimeInNS = MIN_DEAD_TIME;
    MenuTimerUpdate();
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		AnimVarNumberInc(valMain);
		if ( GenConfig.pwmDeadTimeInNS > MAX_DEAD_TIME )
			GenConfig.pwmDeadTimeInNS = MAX_DEAD_TIME;		
    MenuTimerUpdate();
	};
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}

//Menu3 - SubMenu2 - Change Minimum Pulse Time
uint8_t Menu3_SubMenu2_ChangeMinPulseTimeDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		MenuFloatingStrDraw(&Line1BufferNew[0], locLanguageData[22], 1, 16, &menu_iteration_int1);
    //2 строка
		_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[21], 12);
	  snprintf(displayString, 17, "%d", GenConfig.pwmMinPulseLengthInNS);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 10);
		return RESULT_OK;
	};
	
	//проверка полей ввода, создание при необходимости
	if ( !valMain )
	{
		valMain = CreateAnimVarNumber(&Display, 2, 10, &GenConfig.pwmMinPulseLengthInNS, 0, 0);	
    AnimVarNumberEnable(valMain, 1);			
	}
	
	//1 строка
	if ( !( frameNum % 25 ) )
	{
		_clearDisplayBuffer1();
		MenuFloatingStrDraw(&Line1Buffer[0], locLanguageData[22], 1, 16, &menu_iteration_int1);	
	};
	
	//2 строка
	_clearDisplayBuffer2();
	_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[21], 12);
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	
	//выводим числа
	AnimVarNumberDraw(valMain);			
	AnimVarNumberDrawCursor(valMain);
	
	return RESULT_OK;
}

uint8_t Menu3_SubMenu2_ChangeMinPulseTimeEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	//событие при удерживании нажатой кнопки валкодера и поворота по ч стрелке
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CW ) )
	{
		AnimVarNumberCursorMoveRight(valMain);
		return RESULT_OK;
	};
	
	//событие при удерживании нажатой кнопки валкодера и поворота против ч стрелки
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CCW ) )
	{
		AnimVarNumberCursorMoveLeft(valMain);
		return RESULT_OK;
	};
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		AnimVarNumberDec(valMain);
		if ( GenConfig.pwmMinPulseLengthInNS < MIN_PULSE_TIME )
			GenConfig.pwmMinPulseLengthInNS = MIN_PULSE_TIME;
    MenuTimerUpdate();
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		AnimVarNumberInc(valMain);
		if ( GenConfig.pwmMinPulseLengthInNS > MAX_PULSE_TIME )
			GenConfig.pwmMinPulseLengthInNS = MAX_PULSE_TIME;		
    MenuTimerUpdate();
	};
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}

//Menu3 - SubMenu3 - Change Signal Type
uint8_t Menu3_SubMenu3_ChangeSignalTypeDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		MenuFloatingStrDraw(&Line1BufferNew[0], locLanguageData[23], 1, 16, &menu_iteration_int1);	
	  //2 строка
	  switch ( GenConfig.signalType )
		{
			case signalSinus:
				_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[24],  GetCenterPos(locLanguageData[24]));
		  break;
			
			case signalTriangle:
				_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[25], GetCenterPos(locLanguageData[25]));
		  break;
			
			case signalSquare:
				_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[26], GetCenterPos(locLanguageData[26]));
		  break;
	   };
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//1 строка
	if ( !( frameNum % 25 ) )
	{
		_clearDisplayBuffer1();
		MenuFloatingStrDraw(&Line1Buffer[0], locLanguageData[23], 1, 16, &menu_iteration_int1);	
	};
		
	//2 строка
	_clearDisplayBuffer2();
	switch ( GenConfig.signalType )
	{
		case signalSinus:
			_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[24],  GetCenterPos(locLanguageData[24]));
		  bufferVal = strlen(locLanguageData[24]);
		  bufferVarK = GetCenterPos(locLanguageData[24]);
		  MenuAnimSelectionDraw(&Line2Buffer[0], bufferVarK - 1, bufferVarK - 1,
                            bufferVarK + bufferVal, 16, GenMenu->MenuTargetDrawFPS, frameNum); 
		break;
		
		case signalTriangle:
			_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[25], GetCenterPos(locLanguageData[25]));
		  bufferVal = strlen(locLanguageData[25]);
		  bufferVarK = GetCenterPos(locLanguageData[25]);
		  MenuAnimSelectionDraw(&Line2Buffer[0], bufferVarK - 1, bufferVarK - 1,
                            bufferVarK + bufferVal, 16, GenMenu->MenuTargetDrawFPS, frameNum); 
		break;
		
		case signalSquare:
			_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[26], GetCenterPos(locLanguageData[26]));
		  bufferVal = strlen(locLanguageData[26]);
		  bufferVarK = GetCenterPos(locLanguageData[26]);
		  MenuAnimSelectionDraw(&Line2Buffer[0], bufferVarK - 1, bufferVarK - 1,
                            bufferVarK + bufferVal, 16, GenMenu->MenuTargetDrawFPS, frameNum); 
		break;
	};
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

uint8_t Menu3_SubMenu3_ChangeSignalTypeEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		bufferVal = GenConfig.signalType;
		if ( --GenConfig.signalType < signalIndexMin ) GenConfig.signalType = signalIndexMin;
		if ( ( bufferVal == signalSquare ) && ( GenConfig.signalType != signalSquare ) )
		{
			GenChangeMenu();
			MenuResetSignalVal();
		}
		if ( ( bufferVal != signalSquare ) && ( GenConfig.signalType == signalSquare ) )
		{
			GenChangeMenu();
			MenuResetSignalVal();
		}
		MenuTimerUpdate();
		LedUpdate();
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		bufferVal = GenConfig.signalType;
		if ( ++GenConfig.signalType > signalIndexMax ) GenConfig.signalType = signalIndexMax;
		if ( ( bufferVal == signalSquare ) && ( GenConfig.signalType != signalSquare ) )
		{
			GenChangeMenu();
			MenuResetSignalVal();
		}
		if ( ( bufferVal != signalSquare ) && ( GenConfig.signalType == signalSquare ) )
		{
			GenChangeMenu();
			MenuResetSignalVal();
		}
		MenuTimerUpdate();
		LedUpdate();
	};
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}
	
//Menu3 - SubMenu4 - Change Update Type
uint8_t Menu3_SubMenu4_ChangeUpdateTypeDraw(const uint8_t frameNum)
{	
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		MenuFloatingStrDraw(&Line1BufferNew[0], locLanguageData[27], 1, 16, &menu_iteration_int1);	
	  //2 строка
	  if ( GenConfig.isImmediateUpdate )
	  {
			_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[28],  GetCenterPos(locLanguageData[28]));
	  } else
	  {
			_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[29],  GetCenterPos(locLanguageData[29]));
	  };
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//1 строка
	if ( !( frameNum % 25 ) )
	{
		_clearDisplayBuffer1();
		MenuFloatingStrDraw(&Line1Buffer[0], locLanguageData[27], 1, 16, &menu_iteration_int1);	
	};
		
	//2 строка
	_clearDisplayBuffer2();
	if ( GenConfig.isImmediateUpdate )
	{
		_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[28],  GetCenterPos(locLanguageData[28]));
		bufferVal = strlen(locLanguageData[28]);
		bufferVarK = GetCenterPos(locLanguageData[28]);
		MenuAnimSelectionDraw(&Line2Buffer[0], bufferVarK - 1, bufferVarK - 1,
                          bufferVarK + bufferVal, 16, GenMenu->MenuTargetDrawFPS, frameNum); 
	} else
	{
		_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[29],  GetCenterPos(locLanguageData[29]));
		bufferVal = strlen(locLanguageData[29]);
		bufferVarK = GetCenterPos(locLanguageData[29]);
		MenuAnimSelectionDraw(&Line2Buffer[0], bufferVarK - 1, bufferVarK - 1,
                          bufferVarK + bufferVal, 16, GenMenu->MenuTargetDrawFPS, frameNum); 
	};
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

uint8_t Menu3_SubMenu4_ChangeUpdateTypeEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
	};	
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		GenConfig.isImmediateUpdate = 0;
    LedUpdate();
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		GenConfig.isImmediateUpdate = 1;
    MenuTimerUpdate();
		LedUpdate();
	};
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}

//Menu3 - SubMenu5 - Change Show Real Freq Type
uint8_t Menu3_SubMenu5_ChangeShowFreqTypeDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		MenuFloatingStrDraw(&Line1BufferNew[0], locLanguageData[30], 1, 16, &menu_iteration_int1);	
	  //2 строка
	  switch ( GenConfig.ShowFreqType )
		{
			case ShowFreqTypeNormal:
				_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[31],  GetCenterPos(locLanguageData[31]));
		  break;
			
			case ShowFreqTypeActualFreq:
				_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[32], GetCenterPos(locLanguageData[32]));
		  break;
			
			case ShowFreqTypeDirectControl:
				_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[33], GetCenterPos(locLanguageData[33]));
		  break;
	   };
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//1 строка
	if ( !( frameNum % 25 ) )
	{
		_clearDisplayBuffer1();
		MenuFloatingStrDraw(&Line1Buffer[0], locLanguageData[30], 1, 16, &menu_iteration_int1);	
	};
		
	//2 строка
	_clearDisplayBuffer2();
	switch ( GenConfig.ShowFreqType )
	{
		case ShowFreqTypeNormal:
			_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[31],  GetCenterPos(locLanguageData[31]));
		  bufferVal = strlen(locLanguageData[31]);
		  bufferVarK = GetCenterPos(locLanguageData[31]);
		  MenuAnimSelectionDraw(&Line2Buffer[0], bufferVarK - 1, bufferVarK - 1,
                            bufferVarK + bufferVal, 16, GenMenu->MenuTargetDrawFPS, frameNum); 
		break;
		
		case ShowFreqTypeActualFreq:
			_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[32], GetCenterPos(locLanguageData[32]));
		  bufferVal = strlen(locLanguageData[32]);
		  bufferVarK = GetCenterPos(locLanguageData[32]);
		  MenuAnimSelectionDraw(&Line2Buffer[0], bufferVarK - 1, bufferVarK - 1,
                            bufferVarK + bufferVal, 16, GenMenu->MenuTargetDrawFPS, frameNum); 
		break;
		
		case ShowFreqTypeDirectControl:
			_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[33], GetCenterPos(locLanguageData[33]));
		  bufferVal = strlen(locLanguageData[33]);
		  bufferVarK = GetCenterPos(locLanguageData[33]);
		  MenuAnimSelectionDraw(&Line2Buffer[0], bufferVarK - 1, bufferVarK - 1,
                            bufferVarK + bufferVal, 16, GenMenu->MenuTargetDrawFPS, frameNum); 
		break;
	};
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

uint8_t Menu3_SubMenu5_ChangeShowFreqTypeEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		MenuGoToNextItem(GenMenu);
	};	
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		bufferVal = GenConfig.ShowFreqType;
		if ( --GenConfig.ShowFreqType < ShowFreqTypeNormal ) GenConfig.ShowFreqType = ShowFreqTypeNormal;
		if ( bufferVal != GenConfig.ShowFreqType )
		{
			GenChangeMenu();
			MenuTimerUpdate();
		}
		LedUpdate();
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		bufferVal = GenConfig.ShowFreqType;
		if ( ++GenConfig.ShowFreqType > ShowFreqTypeDirectControl ) GenConfig.ShowFreqType = ShowFreqTypeDirectControl;
		if ( bufferVal != GenConfig.ShowFreqType )
		{
			GenChangeMenu();
			MenuTimerUpdate();
		}
		LedUpdate();
	};
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}

//Menu3 - SubMenu6 - Change Language
uint8_t Menu3_SubMenu6_ChangeLanguageDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		MenuFloatingStrDraw(&Line1BufferNew[0], locLanguageData[34], 1, 16, &menu_iteration_int1);
	  //2 строка
		switch ( GenConfig.languageId )
		{
			case en_US:
				_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[35],  GetCenterPos(locLanguageData[35]));
			break;
			
			case ru_RU:
				_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[36],  GetCenterPos(locLanguageData[36]));
			break;
		};
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//1 строка
	if ( !( frameNum % 25 ) )
	{
		_clearDisplayBuffer1();
		MenuFloatingStrDraw(&Line1Buffer[0], locLanguageData[34], 1, 16, &menu_iteration_int1);	
	};
		
	//2 строка
	_clearDisplayBuffer2();
  switch ( GenConfig.languageId )
	{
		case en_US:
			_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[35],  GetCenterPos(locLanguageData[35]));
		  bufferVal = strlen(locLanguageData[35]);
		  bufferVarK = GetCenterPos(locLanguageData[35]);
		  MenuAnimSelectionDraw(&Line2Buffer[0], bufferVarK - 1, bufferVarK - 1,
                            bufferVarK + bufferVal, 16, GenMenu->MenuTargetDrawFPS, frameNum); 
		break;
			
		case ru_RU:
		  _copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[36],  GetCenterPos(locLanguageData[36]));
		  bufferVal = strlen(locLanguageData[36]);
		  bufferVarK = GetCenterPos(locLanguageData[36]);
		  MenuAnimSelectionDraw(&Line2Buffer[0], bufferVarK - 1, bufferVarK - 1,
                            bufferVarK + bufferVal, 16, GenMenu->MenuTargetDrawFPS, frameNum); 
		break;
	};
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

uint8_t Menu3_SubMenu6_ChangeLanguageEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		FreeDataWhenTransition();
		menuTransDirection = 1; //вправо(-)
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		MenuGoToParentItem(GenMenu);
	};	
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		if ( GenConfig.languageId > locLangMin ) GenConfig.languageId--;
		GenChangeLocalization(GenConfig.languageId);
  };
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		if ( GenConfig.languageId < locLangMax ) GenConfig.languageId++;
		GenChangeLocalization(GenConfig.languageId);
	};
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}

//Menu4 - SubMenu1 - Save Dialog
uint8_t Menu4_SubMenu1_SaveDialogDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
	  //1 строка
		_copyStringToBufferLtoR(&Line1BufferNew[0], locLanguageData[37],  GetCenterPos(locLanguageData[37]));	
	  //2 строка
		_copyStringToBufferLtoR(&Line2BufferNew[0], locLanguageData[38],  GetCenterPos(locLanguageData[38]));	
		return RESULT_OK;
	};
	
	if ( !bufferVal )
	{
		//пауза 1 мс
		delayMS(1000);
		//очистка
	  _clearDisplayBuffers();
	  //1 строка
		_copyStringToBufferLtoR(&Line1Buffer[0], locLanguageData[37],  GetCenterPos(locLanguageData[37]));	
	  //2 строка
		//сохранение
		if ( FlashWriteData( (uint8_t*)&GenConfig, sizeof(GenConfig) ) )
		{
			_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[39],  GetCenterPos(locLanguageData[39]));
		} else
		{
			_copyStringToBufferLtoR(&Line2Buffer[0], locLanguageData[40],  GetCenterPos(locLanguageData[40]));
		};		
	};
	
	//в bufferVal инкрементируем количество кадров
	bufferVal++;
	//при bufferVal = ( 3 сек ) выходим
	if ( ( bufferVal * ( 1000 / GenMenu->MenuTargetDrawFPS ) ) > 3000 )
	{
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода
		menuTransDirection = 1; //вправо(-)
		MenuGoToParentItem(GenMenu);
	};	
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

