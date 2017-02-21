
#include "settings.h"
#include "gen_menu.h"
#include "gen_system.h"
#include "gen_timers.h"
#include "hd44780display.h"
#include "bsc_tree_menu.h"
#include "bsc_stm32_delay.h"
#include "gen_flash.h"
#include "bsc_anim_ctrls.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//значения дельты приращения частот при повороте ручки валкодера
#define DeltaValMinValue 1
#define DeltaValMaxValue 1000

extern HD44780_DISPLAY_STRUCT Display;
extern TREE_MENU *GenMenu;
extern uint16_t MenuOKItemID;
extern struct MainConfig
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

char Line1Buffer[16] = "                ";
char Line2Buffer[16] = "                ";

ANIM_VAR_NUMBER *valMain;
ANIM_VAR_NUMBER *valIncSize;
int32_t incSize;

//глобальные переменные для хранения промежуточных значений
//размещаем сразу, чтобы не тратить время на выделение памяти
//указатель на изменяемое число (на 1 строке или на 2)
uint8_t cursorNumN;
char displayString[16];
//направление перехода меню влево(+) - 2, вправо(-) - 1
uint8_t menuTransDirection = 0;
int32_t bufferVal = 0;
int32_t bufferVarK = 0;
double bufferValDouble = 0;
int16_t menu_iteration = 0;
char Line1BufferNew[16] = "                ";
char Line2BufferNew[16] = "                ";
uint16_t LastMenuItemID = 0;

void FreeDataWhenTransition(void)
{
	DestroyAnimVarNumber(&valMain);
	DestroyAnimVarNumber(&valIncSize);
	incSize = 1;
	cursorNumN = 1;
	bufferVal = 0;
	bufferVarK = 0;
	menu_iteration = 0;
}

void LedUpdate(void)
{
	//светодиоды для индикации
	if ( GenConfig.isImmediateUpdate ) LED_BLUE_ON; else LED_BLUE_OFF;
	if ( GenConfig.signalType == 2 ) LED_GREEN_ON; else LED_GREEN_OFF;
}

void Buttons_1_2_scan(SYS_EVENTS_DATA genEvents)
{
	if ( genEvents & EVENT_BUTTON1_CLICK )
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
	}
	
	if ( genEvents & EVENT_BUTTON2_CLICK )
	{
		UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                 GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		
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
}

void _clearDisplayNewBuffers(void)
{
	uint8_t i;
	for ( i = 0; i < 16; i++ )
	{
		Line1BufferNew[i] = ' ';
		Line2BufferNew[i] = ' ';
	}
}

void _copyStringToBufferLtoR(char *_buffer, char *_string, uint8_t _posL)
{
	uint8_t i = 0;
	while ( ( ( _posL + i ) < 17 ) && ( _string[i] ) )
	{
		_buffer[_posL + i - 1] = _string[i];
		i++;
	}
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
}


void MenuTransitionDraw(const uint16_t frameNum)
{	
	uint8_t i = 0;
	//_clearDisplayBuffers();
	
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
	_copyStringToBufferLtoR(&Line1Buffer[0], "Hi!", 7);	
	//2 строка
	_copyStringToBufferLtoR(&Line2Buffer[0], "****************", 1);
	
	//выходим после 1с
	if ( frameNum >= GenMenu->MenuTargetDrawFPS )
	{
		menuTransDirection = 2; //влево(+)

		Menu1_StartMenuDraw(0);
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
	  _copyStringToBufferLtoR(&Line1BufferNew[0], "OK", 7);
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
	_copyStringToBufferLtoR(&Line1Buffer[0], "OK", 7);
	
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
	  _copyStringToBufferLtoR(&Line1BufferNew[0], "F PWM = ", 1);
	  snprintf(displayString, 17, "%d", GenConfig.freqPWM);
	  _copyStringToBufferRtoL(&Line1BufferNew[0], displayString, strlen(displayString), 16);
	  _copyStringToBufferLtoR(&Line2BufferNew[0], "F Sig = ", 1);
	  snprintf(displayString, 17, "%d", GenConfig.freqSignal);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 16);
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	_clearDisplayBuffers();
	_copyStringToBufferLtoR(&Line1Buffer[0], "F PWM = ", 1);
	snprintf(displayString, 17, "%d", GenConfig.freqPWM);
	_copyStringToBufferRtoL(&Line1Buffer[0], displayString, strlen(displayString), 16);
	_copyStringToBufferLtoR(&Line2Buffer[0], "F Sig = ", 1);
	snprintf(displayString, 17, "%d", GenConfig.freqSignal);
	_copyStringToBufferRtoL(&Line2Buffer[0], displayString, strlen(displayString), 16);
	
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

		Menu2_MainMenuDraw(0);
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
	  _copyStringToBufferLtoR(&Line1BufferNew[0], "Main config ~", 3);	
	  //2 строка
    _copyStringToBufferLtoR(&Line2BufferNew[0], " Press Щ or Ъ button to enter to MAIN config ", 1);
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//очистка
	//_clearDisplayBuffers(); без очистки
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], "Main config ~", 3);
	//2 строка
	if ( !( frameNum % 25 ) )
	{
		MenuTickerDraw(&Line2Buffer[0], " Press Щ or Ъ button to enter to MAIN config ", 16, &menu_iteration);
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

		Menu2_SubMenu1_ChangePWMFreqDraw(0);
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
		
		Menu1_StartMenuDraw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода		
		menuTransDirection = 2; //влево(+)

		Menu3_ExtraMenuDraw(0);
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
	  _copyStringToBufferLtoR(&Line1BufferNew[0], "Extra config ~", 2);	
	  //2 строка
    _copyStringToBufferLtoR(&Line2BufferNew[0], " Press Щ or Ъ button to enter to EXTRA config ", 1);
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//очистка
	//_clearDisplayBuffers(); без очистки
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], "Extra config ~", 2);
	//2 строка
	if ( !( frameNum % 25 ) )
	{
		MenuTickerDraw(&Line2Buffer[0], " Press Щ or Ъ button to enter to EXTRA config ", 16, &menu_iteration);
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

		Menu3_SubMenu1_ChangeDeadTimeDraw(0);
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
		
		Menu2_MainMenuDraw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода		
		menuTransDirection = 2; //влево(+)

		Menu4_SaveMenuDraw(0);
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
	  _copyStringToBufferLtoR(&Line1BufferNew[0], "Save config?", 3);	
	  //2 строка
    _copyStringToBufferLtoR(&Line2BufferNew[0], " Press Щ or Ъ button for save ", 1);
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//очистка
	//_clearDisplayBuffers(); без очистки
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], "Save config?", 3);
	//2 строка
	if ( !( frameNum % 25 ) )
	{
		MenuTickerDraw(&Line2Buffer[0], " Press Щ or Ъ button for save ", 16, &menu_iteration);
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
		
		Menu3_ExtraMenuDraw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//нажатие 3 или 4
	if ( ( genEvents & EVENT_BUTTON3_CLICK ) || ( genEvents & EVENT_BUTTON4_CLICK ) )
	{
		//очищаем
		FreeDataWhenTransition();
		//уст направление перехода
		menuTransDirection = 2; //влево(+)
		
		Menu4_SubMenu1_SaveDialogDraw(0);
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
		_copyStringToBufferLtoR(&Line1BufferNew[0], "F PWM = ", 1);
		snprintf(displayString, 17, "%d", GenConfig.freqPWM);
	  _copyStringToBufferRtoL(&Line1BufferNew[0], displayString, strlen(displayString), 16);
    //2 строка
	  _copyStringToBufferLtoR(&Line2BufferNew[0], "step = ", 2);
	  snprintf(displayString, 17, "%d", incSize);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 16);
		return RESULT_OK;
	};
	
	//проверка полей ввода, создание при необходимости
	if ( ( !valMain ) && ( !valIncSize ) )
	{
		incSize = 1;
		valMain = CreateAnimVarNumber(&Display, 1, 16, &GenConfig.freqPWM, 0, &incSize);
		valIncSize = CreateAnimVarNumber(&Display, 2, 16, &incSize, 0, 0);	
    AnimVarNumberEnable(valMain, 1);	
    cursorNumN = 1;		
	}
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], "F PWM = ", 1);	
	//2 строка
	_copyStringToBufferLtoR(&Line2Buffer[0], "step = ", 2);
	
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
		
		Menu2_MainMenuDraw(0);
		MenuGoToParentItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		
		Menu2_SubMenu2_ChangeSignalFreqDraw(0);
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		switch ( cursorNumN )
		{
			case 1:
				bufferVal = GenConfig.freqPWM;
				AnimVarNumberDec(valMain);
				if ( ( GenConfig.freqPWM > 0 ) && FrequencyCheck(GenConfig.freqPWM, GenConfig.freqSignal) )
				{
					if ( GenConfig.isImmediateUpdate )
						UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                         GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		    } else
				{
					GenConfig.freqPWM = bufferVal;
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
				bufferVal = GenConfig.freqPWM;
				AnimVarNumberInc(valMain);
				if ( ( GenConfig.freqPWM > 0 ) && FrequencyCheck(GenConfig.freqPWM, GenConfig.freqSignal) )
				{
					if ( GenConfig.isImmediateUpdate )
						UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                         GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		    } else
				{
					GenConfig.freqPWM = bufferVal;
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
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}

//Menu2 - SubMenu2 - Change Signal Freq
uint8_t Menu2_SubMenu2_ChangeSignalFreqDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		_copyStringToBufferLtoR(&Line1BufferNew[0], "F sign = ", 1);
		snprintf(displayString, 17, "%d", GenConfig.freqSignal);
	  _copyStringToBufferRtoL(&Line1BufferNew[0], displayString, strlen(displayString), 16);
    //2 строка
	  _copyStringToBufferLtoR(&Line2BufferNew[0], "step = ", 3);
	  snprintf(displayString, 17, "%d", incSize);
	  _copyStringToBufferRtoL(&Line2BufferNew[0], displayString, strlen(displayString), 16);
		return RESULT_OK;
	};
	
	//проверка полей ввода, создание при необходимости
	if ( ( !valMain ) && ( !valIncSize ) )
	{
		incSize = 1;
		valMain = CreateAnimVarNumber(&Display, 1, 16, &GenConfig.freqSignal, 0, &incSize);
		valIncSize = CreateAnimVarNumber(&Display, 2, 16, &incSize, 0, 0);	
    AnimVarNumberEnable(valMain, 1);	
    cursorNumN = 1;		
	}
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], "F sign = ", 1);	
	//2 строка
	_copyStringToBufferLtoR(&Line2Buffer[0], "step = ", 3);
	
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
		
		Menu2_SubMenu1_ChangePWMFreqDraw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		
		Menu2_SubMenu3_ChangeSignalPowerDraw(0);
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		switch ( cursorNumN )
		{
			case 1:
				bufferVal = GenConfig.freqSignal;
				AnimVarNumberDec(valMain);
				if ( ( GenConfig.freqSignal > 0 ) && FrequencyCheck(GenConfig.freqPWM, GenConfig.freqSignal) )
				{
					if ( GenConfig.isImmediateUpdate )
						UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                         GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		    } else
				{
					GenConfig.freqSignal = bufferVal;
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
				bufferVal = GenConfig.freqSignal;
				AnimVarNumberInc(valMain);
				if ( ( GenConfig.freqSignal > 0 ) && FrequencyCheck(GenConfig.freqPWM, GenConfig.freqSignal) )
				{
					if ( GenConfig.isImmediateUpdate )
						UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                         GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		    } else
				{
					GenConfig.freqSignal = bufferVal;
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
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}

//Menu2 - SubMenu3 - Change Signal Power
uint8_t Menu2_SubMenu3_ChangeSignalPowerDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		_copyStringToBufferLtoR(&Line1BufferNew[0], "Power factor", 3);
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
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], "Power factor", 3);	
	
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
		
		Menu2_SubMenu2_ChangeSignalFreqDraw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		
		Menu2_SubMenu4_ChangeSignalCenterDraw(0);
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		AnimVarNumberDec(valMain);
		if ( GenConfig.powerK < MIN_POWER_K )
			GenConfig.powerK = MIN_POWER_K;
		if ( GenConfig.isImmediateUpdate )
			UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                   GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		AnimVarNumberInc(valMain);
		if ( GenConfig.powerK > MAX_POWER_K )
			GenConfig.powerK = MAX_POWER_K;
		if ( GenConfig.isImmediateUpdate )
			UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                   GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
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
		_copyStringToBufferLtoR(&Line1BufferNew[0], "Center factor", 2);
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
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], "Center factor", 2);	
	
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
		
		Menu2_SubMenu3_ChangeSignalPowerDraw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		
		Menu2_MainMenuDraw(0);
		MenuGoToParentItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		AnimVarNumberDec(valMain);
		if ( GenConfig.centerK < MIN_CENTER_K )
			GenConfig.centerK = MIN_CENTER_K;
		if ( GenConfig.isImmediateUpdate )
			UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                   GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		AnimVarNumberInc(valMain);
		if ( GenConfig.centerK > MAX_CENTER_K )
			GenConfig.centerK = MAX_CENTER_K;
		if ( GenConfig.isImmediateUpdate )
			UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                   GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
	};
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}

//Menu3 - SubMenu1 - Change Dead Time
uint8_t Menu3_SubMenu1_ChangeDeadTimeDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		_copyStringToBufferLtoR(&Line1BufferNew[0], "Dead Time", 4);
    //2 строка
		_copyStringToBufferLtoR(&Line2BufferNew[0], "ns", 12);
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
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], "Dead Time", 4);	
	//2 строка
	_copyStringToBufferLtoR(&Line2Buffer[0], "ns", 12);
	
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
		
		Menu3_ExtraMenuDraw(0);
		MenuGoToParentItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		
		Menu3_SubMenu2_ChangeMinPulseTimeDraw(0);
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		AnimVarNumberDec(valMain);
		if ( GenConfig.pwmDeadTimeInNS < MIN_DEAD_TIME )
			GenConfig.pwmDeadTimeInNS = MIN_DEAD_TIME;
		if ( GenConfig.isImmediateUpdate )
			UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                   GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		AnimVarNumberInc(valMain);
		if ( GenConfig.pwmDeadTimeInNS > MAX_DEAD_TIME )
			GenConfig.pwmDeadTimeInNS = MAX_DEAD_TIME;		
		if ( GenConfig.isImmediateUpdate )
			UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                   GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
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
		_copyStringToBufferLtoR(&Line1BufferNew[0], "Min pulse time", 2);
    //2 строка
		_copyStringToBufferLtoR(&Line2BufferNew[0], "ns", 12);
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
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], "Min pulse time", 2);	
	//2 строка
	_copyStringToBufferLtoR(&Line2Buffer[0], "ns", 12);
	
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
		
		Menu3_SubMenu1_ChangeDeadTimeDraw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		
		Menu3_SubMenu3_ChangeSignalTypeDraw(0);
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		AnimVarNumberDec(valMain);
		if ( GenConfig.pwmMinPulseLengthInNS < MIN_PULSE_TIME )
			GenConfig.pwmMinPulseLengthInNS = MIN_PULSE_TIME;
		if ( GenConfig.isImmediateUpdate )
			UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                   GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		AnimVarNumberInc(valMain);
		if ( GenConfig.pwmMinPulseLengthInNS > MAX_PULSE_TIME )
			GenConfig.pwmMinPulseLengthInNS = MAX_PULSE_TIME;		
		if ( GenConfig.isImmediateUpdate )
			UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                   GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
	};
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}

//Menu3 - SubMenu3 - Change Minimum Pulse Time
uint8_t Menu3_SubMenu3_ChangeSignalTypeDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
	  _copyStringToBufferLtoR(&Line1BufferNew[0], "Signal  type", 3);	
	  //2 строка
	  switch ( GenConfig.signalType )
		{
			case 1:
				_copyStringToBufferLtoR(&Line2BufferNew[0], "Sinus", 6);
		  break;
			
			case 2:
				_copyStringToBufferLtoR(&Line2BufferNew[0], "Triangle", 5);
		  break;
	   };
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], "Signal  type", 3);	
	//2 строка
	switch ( GenConfig.signalType )
	{
		case 1:
			_copyStringToBufferLtoR(&Line2Buffer[0], "Sinus", 6);
		  MenuAnimSelectionDraw(&Line2Buffer[0], 5, 5, 11, 16, GenMenu->MenuTargetDrawFPS, frameNum); 
		  Line2Buffer[15] = 0xC8;
		break;
		
		case 2:
			_copyStringToBufferLtoR(&Line2Buffer[0], "Triangle", 5);
		  MenuAnimSelectionDraw(&Line2Buffer[0], 4, 4, 13, 16, GenMenu->MenuTargetDrawFPS, frameNum);
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
		
		Menu3_SubMenu2_ChangeMinPulseTimeDraw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		
		Menu3_SubMenu4_ChangeUpdateTypeDraw(0);
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		if ( !--GenConfig.signalType ) GenConfig.signalType = 1;
		if ( GenConfig.isImmediateUpdate )
			UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                   GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		LedUpdate();
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		if ( ++GenConfig.signalType > 2 ) GenConfig.signalType = 2;
		if ( GenConfig.isImmediateUpdate )
			UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                   GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
	  LedUpdate();
	};
	
	Buttons_1_2_scan(genEvents);
	
	return RESULT_OK;
}
	
//Menu3 - SubMenu4 - Change Minimum Pulse Time
uint8_t Menu3_SubMenu4_ChangeUpdateTypeDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
	  _copyStringToBufferLtoR(&Line1BufferNew[0], "Signal  update", 2);	
	  //2 строка
	  if ( GenConfig.isImmediateUpdate )
	  {
		  _copyStringToBufferLtoR(&Line2BufferNew[0], "Auto", 7);
	  } else
	  {
		  _copyStringToBufferLtoR(&Line2BufferNew[0], "Manually", 5);
	  };
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBufferLtoR(&Line1Buffer[0], "Signal  update", 2);	
	//2 строка
	if ( GenConfig.isImmediateUpdate )
	{
		_copyStringToBufferLtoR(&Line2Buffer[0], "Auto", 7);
		MenuAnimSelectionDraw(&Line2Buffer[0], 6, 6, 11, 16, GenMenu->MenuTargetDrawFPS, frameNum);
	} else
	{
		_copyStringToBufferLtoR(&Line2Buffer[0], "Manually", 5);
		MenuAnimSelectionDraw(&Line2Buffer[0], 4, 4, 13, 16, GenMenu->MenuTargetDrawFPS, frameNum);
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
		
		Menu3_SubMenu3_ChangeSignalTypeDraw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		FreeDataWhenTransition();
		menuTransDirection = 2; //влево(+)
		
		Menu3_ExtraMenuDraw(0);
		MenuGoToParentItem(GenMenu);
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
		UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, (double)GenConfig.powerK / 100, (double)GenConfig.centerK / 100,
                 GenConfig.pwmMinPulseLengthInNS, GenConfig.pwmDeadTimeInNS, GenConfig.signalType);
		LedUpdate();
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
	  _copyStringToBufferLtoR(&Line1BufferNew[0], "Config", 6);	
	  //2 строка
    _copyStringToBufferLtoR(&Line2BufferNew[0], "......", 6);
		return RESULT_OK;
	};
	
	if ( !bufferVal )
	{
		//пауза 1 мс
		delayMS(1000);
		//очистка
	  _clearDisplayBuffers();
	  //1 строка
	  _copyStringToBufferLtoR(&Line1Buffer[0], "Config", 6);	
	  //2 строка
		//сохранение
		if ( FlashWriteData( (uint8_t*)&GenConfig, sizeof(GenConfig) ) )
		{
			_copyStringToBufferLtoR(&Line2Buffer[0], "saved!", 6);
		} else
		{
			_copyStringToBufferLtoR(&Line2Buffer[0], "failed! :(", 4);
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
		
		Menu4_SaveMenuDraw(0);
		MenuGoToParentItem(GenMenu);
	};	
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

