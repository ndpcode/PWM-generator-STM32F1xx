
#include "settings.h"
#include "gen_menu.h"
#include "gen_system.h"
#include "gen_timers.h"
#include "hd44780display.h"
#include "bsc_tree_menu.h"
#include "bsc_stm32_delay.h"
#include "gen_flash.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//значения дельты приращения частот при повороте ручки валкодера
#define DeltaValMinValue 1
#define DeltaValMaxValue 1000

extern HD44780_DISPLAY_STRUCT Display;
extern TREE_MENU *GenMenu;
extern struct MainConfig
{
	unsigned isImmediateUpdate : 1;
	uint32_t freqPWM;
	uint32_t freqSignal;
	double powerK;
	uint8_t signalType;
} GenConfig;

char Line1Buffer[16] = "                ";
char Line2Buffer[16] = "                ";

//глобальные переменные для хранения промежуточных значений
//размещаем сразу, чтобы не тратить время на выделение памяти
//буфер для операции со строками
char displayString[12];
//направление перехода меню влево(+) - 2, вправо(-) - 1
uint8_t menuTransDirection = 0;
//num1 - изменяемая переменная 1 (на 1 строке)
//num2 - изменяемая переменная 2 (на 2 строке)
uint8_t num1CursorPos = 1;
uint8_t num1Length;
uint32_t num2Val = 1;
uint8_t num2CursorPos = 1;
uint8_t num2Length;
uint8_t cursorNumN = 1;
uint32_t bufferVal = 0;
double bufferValDouble = 0;
int16_t menu7_iteration = 0;
char Line1BufferNew[16] = "                ";
char Line2BufferNew[16] = "                ";

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

void _copyStringToBuffer(char *_buffer, char *_string, uint8_t _pos)
{
	uint8_t i = 0;
	while ( ( ( _pos + i ) < 16 ) && ( _string[i] ) )
	{
		_buffer[_pos+i] = _string[i];
		i++;
	}
}

uint32_t getDeltaNumFromCursorPos(uint8_t _cursorPos)
{
	uint32_t result = 1;
	while ( --_cursorPos )
		result *= 10;
	return result;		
}

void MenuAnimSelectionDraw(char *_buffer, uint8_t length, uint8_t leftBorder, uint8_t rightBorder, uint8_t frameAll, uint8_t frameNum)
{
	uint8_t i = 0;
	if ( ( leftBorder >= length ) && ( ( rightBorder + length - 1 ) < 16 ) )
		for ( i = 0; i < length; i++ )
	  {
		  if ( frameNum >= ( i * ( frameAll / length ) ) )
		  {
			  _buffer[leftBorder - length + i] = '>';
	      _buffer[rightBorder + length - i - 1] = '<';
		  };
	  };
}

void MenuTickerDraw(char *_buffer, char *tickerText, int16_t *iteration)
{
	uint8_t i = 0;
	uint8_t tickerLength = strlen(tickerText);
	if ( ( abs(*iteration) + 16 ) <= tickerLength )
	{
		for ( i = 0; i < 16; i++ )
		{
			_buffer[i] = tickerText[i + abs(*iteration)];		
		}
	};
	(*iteration)++;
  if ( ( abs(*iteration) + 16 ) > tickerLength ) *iteration = -(*iteration - 2);	
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

uint8_t Menu1Draw(const uint8_t frameNum)
{	
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
	  _copyStringToBuffer(&Line1BufferNew[0], "F PWM = ", 1);
	  snprintf(displayString, 12, "%d", GenConfig.freqPWM);
	  _copyStringToBuffer(&Line1BufferNew[0], displayString, 9);
	  _copyStringToBuffer(&Line2BufferNew[0], "F Sig = ", 1);
	  snprintf(displayString, 12, "%d", GenConfig.freqSignal);
	  _copyStringToBuffer(&Line2BufferNew[0], displayString, 9);
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	_clearDisplayBuffers();
	_copyStringToBuffer(&Line1Buffer[0], "F PWM = ", 1);
	snprintf(displayString, 12, "%d", GenConfig.freqPWM);
	_copyStringToBuffer(&Line1Buffer[0], displayString, 9);
	_copyStringToBuffer(&Line2Buffer[0], "F Sig = ", 1);
	snprintf(displayString, 12, "%d", GenConfig.freqSignal);
	_copyStringToBuffer(&Line2Buffer[0], displayString, 9);
	
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

uint8_t Menu1Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		num1CursorPos = 1;
		snprintf(displayString, 12, "%d", GenConfig.freqPWM);
		num1Length = strlen(displayString);
		num2Val = 1;
		num2CursorPos = 1;
    num2Length = 1;
		cursorNumN = 1;
		menuTransDirection = 2; //влево(+)

		Menu2Draw(0);
		MenuGoToNextItem(GenMenu);
  };
	return RESULT_OK;
}

uint8_t Menu2Draw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		_copyStringToBuffer(&Line1BufferNew[0], "F PWM = ", 1);
		snprintf(displayString, 12, "%d", GenConfig.freqPWM);
		_copyStringToBuffer(&Line1BufferNew[0], displayString, 9);
    //2 строка
	  _copyStringToBuffer(&Line2BufferNew[0], "inc = ", 3);
	  snprintf(displayString, 12, "%d", num2Val);
	  _copyStringToBuffer(&Line2BufferNew[0], displayString, 9);
		return RESULT_OK;
	};
	
	if ( ! (Display.displayOnOffControl & HD44780_FLAG_CURSORON) ) HD44780DisplaySetCursorVisible(&Display, 1);
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBuffer(&Line1Buffer[0], "F PWM = ", 1);
	snprintf(displayString, 12, "%d", GenConfig.freqPWM);
	_copyStringToBuffer(&Line1Buffer[0], displayString, 9);	
	//2 строка
	_copyStringToBuffer(&Line2Buffer[0], "inc = ", 3);
	snprintf(displayString, 12, "%d", num2Val);
	_copyStringToBuffer(&Line2Buffer[0], displayString, 9);
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	//позиция курсора
	switch ( cursorNumN )
	{
		case 1:
			HD44780DisplayMoveCursor(&Display, 1, 10 + num1Length - num1CursorPos);
		break;
		case 2:
			HD44780DisplayMoveCursor(&Display, 2, 10 + num2Length - num2CursorPos);
		break;
	};
	return RESULT_OK;
}

uint8_t Menu2Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	//событие при удерживании нажатой кнопки валкодера и поворота по ч стрелке
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CW ) )
	{
		switch ( cursorNumN )
		{
			case 1:
				if ( num1CursorPos > 1 ) num1CursorPos--;
		  break;
			case 2:
				if ( num2CursorPos > 1 ) num2CursorPos--;
		  break;
	  };
		return RESULT_OK;
	};
	
	//событие при удерживании нажатой кнопки валкодера и поворота против ч стрелки
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CCW ) )
	{
		switch ( cursorNumN )
		{
			case 1:
				if ( num1CursorPos < num1Length ) num1CursorPos++;
		  break;
			case 2:
				if ( num2CursorPos < num2Length ) num2CursorPos++;
		  break;
	  };
		return RESULT_OK;
	};
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		menuTransDirection = 1; //вправо(-)
		
		Menu1Draw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		num1CursorPos = 1;
		snprintf(displayString, 12, "%d", GenConfig.freqSignal);
		num1Length = strlen(displayString);
		num2Val = 1;
		num2CursorPos = 1;
    num2Length = 1;
		cursorNumN = 1;
		menuTransDirection = 2; //влево(+)
		
		Menu3Draw(0);
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		switch ( cursorNumN )
		{
			case 1:
				bufferVal = num2Val * getDeltaNumFromCursorPos(num1CursorPos);
				if ( GenConfig.freqPWM > bufferVal )
					bufferVal = GenConfig.freqPWM - bufferVal;
				if ( FrequencyCheck(bufferVal, GenConfig.freqSignal) )
				{
					GenConfig.freqPWM = bufferVal;
					snprintf(displayString, 12, "%d", GenConfig.freqPWM);
					num1Length = strlen(displayString);
					if ( GenConfig.isImmediateUpdate )
						UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.powerK, GenConfig.signalType);
		    }
		  break;
			case 2:
				bufferVal = getDeltaNumFromCursorPos(num2CursorPos);
				if ( num2Val > bufferVal )
					num2Val -= bufferVal;
				if ( num2Val < DeltaValMinValue )
					num2Val = DeltaValMinValue;
				snprintf(displayString, 12, "%d", num2Val);
				num2Length = strlen(displayString);
		  break;
	  };
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		switch ( cursorNumN )
		{
			case 1:
				bufferVal = GenConfig.freqPWM + num2Val * getDeltaNumFromCursorPos(num1CursorPos);
				if ( FrequencyCheck(bufferVal, GenConfig.freqSignal) )
				{
					GenConfig.freqPWM = bufferVal;
					snprintf(displayString, 12, "%d", GenConfig.freqPWM);
					num1Length = strlen(displayString);
					if ( num1CursorPos > num1Length ) num1CursorPos = num1Length;
					if ( GenConfig.isImmediateUpdate )
						UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.powerK, GenConfig.signalType);
		    }
		  break;
			case 2:
				bufferVal = getDeltaNumFromCursorPos(num2CursorPos);
			  num2Val += bufferVal;
				if ( num2Val > DeltaValMaxValue )
					num2Val = DeltaValMaxValue;
				snprintf(displayString, 12, "%d", num2Val);
				num2Length = strlen(displayString);
				if ( num2CursorPos > num2Length ) num2CursorPos = num2Length;
		  break;
	  };
	};
	
	//событие при нажатии кнопки 3
	if ( genEvents & EVENT_BUTTON3_CLICK )
	{
		cursorNumN--;
		if ( cursorNumN < 1 ) cursorNumN = 1;
	};
	
	//событие при нажатии кнопки 4
	if ( genEvents & EVENT_BUTTON4_CLICK )
	{
		cursorNumN++;
		if ( cursorNumN > 2 ) cursorNumN = 2;
	};
	
	return RESULT_OK;
}

uint8_t Menu3Draw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		_copyStringToBuffer(&Line1BufferNew[0], "F Sign = ", 1);
		snprintf(displayString, 12, "%d", GenConfig.freqSignal);
		_copyStringToBuffer(&Line1BufferNew[0], displayString, 10);
    //2 строка
	  _copyStringToBuffer(&Line2BufferNew[0], "inc = ", 3);
	  snprintf(displayString, 12, "%d", num2Val);
	  _copyStringToBuffer(&Line2BufferNew[0], displayString, 9);
		return RESULT_OK;
	};
	
	if ( ! (Display.displayOnOffControl & HD44780_FLAG_CURSORON) ) HD44780DisplaySetCursorVisible(&Display, 1);
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBuffer(&Line1Buffer[0], "F Sign = ", 1);
	snprintf(displayString, 12, "%d", GenConfig.freqSignal);
	_copyStringToBuffer(&Line1Buffer[0], displayString, 10);	
	//2 строка
	_copyStringToBuffer(&Line2Buffer[0], "inc = ", 3);
	snprintf(displayString, 12, "%d", num2Val);
	_copyStringToBuffer(&Line2Buffer[0], displayString, 9);
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	//позиция курсора
	switch ( cursorNumN )
	{
		case 1:
			HD44780DisplayMoveCursor(&Display, 1, 11 + num1Length - num1CursorPos);
		break;
		case 2:
			HD44780DisplayMoveCursor(&Display, 2, 10 + num2Length - num2CursorPos);
		break;
	};
	return RESULT_OK;
}

uint8_t Menu3Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	//событие при удерживании нажатой кнопки валкодера и поворота по ч стрелке
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CW ) )
	{
		switch ( cursorNumN )
		{
			case 1:
				if ( num1CursorPos > 1 ) num1CursorPos--;
		  break;
			case 2:
				if ( num2CursorPos > 1 ) num2CursorPos--;
		  break;
	  };
		return RESULT_OK;
	};
	
	//событие при удерживании нажатой кнопки валкодера и поворота против ч стрелки
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CCW ) )
	{
		switch ( cursorNumN )
		{
			case 1:
				if ( num1CursorPos < num1Length ) num1CursorPos++;
		  break;
			case 2:
				if ( num2CursorPos < num2Length ) num2CursorPos++;
		  break;
	  };
		return RESULT_OK;
	};
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		num1CursorPos = 1;
		snprintf(displayString, 12, "%d", GenConfig.freqPWM);
		num1Length = strlen(displayString);
		num2Val = 1;
		num2CursorPos = 1;
    num2Length = 1;
		cursorNumN = 1;
		menuTransDirection = 1; //вправо(-)
		
		Menu2Draw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		num1CursorPos = 1;
		snprintf(displayString, 12, "%.2lf", GenConfig.powerK);
		num1Length = strlen(displayString);
		num2Val = 1;
		num2CursorPos = 1;
    num2Length = 1;
		cursorNumN = 1;
		menuTransDirection = 2; //влево(+)
		
		Menu4Draw(0);
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		switch ( cursorNumN )
		{
			case 1:
				bufferVal = num2Val * getDeltaNumFromCursorPos(num1CursorPos);
				if ( GenConfig.freqSignal > bufferVal )
					bufferVal = GenConfig.freqSignal - bufferVal;
				if ( FrequencyCheck(GenConfig.freqPWM, bufferVal) )
				{
					GenConfig.freqSignal = bufferVal;
					snprintf(displayString, 12, "%d", GenConfig.freqSignal);
					num1Length = strlen(displayString);
					if ( GenConfig.isImmediateUpdate )
						UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.powerK, GenConfig.signalType);
		    }
		  break;
			case 2:
				bufferVal = getDeltaNumFromCursorPos(num2CursorPos);
				if ( num2Val > bufferVal )
					num2Val -= bufferVal;
				if ( num2Val < DeltaValMinValue )
					num2Val = DeltaValMinValue;
				snprintf(displayString, 12, "%d", num2Val);
				num2Length = strlen(displayString);
		  break;
	  };
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		switch ( cursorNumN )
		{
			case 1:
				bufferVal = GenConfig.freqSignal + num2Val * getDeltaNumFromCursorPos(num1CursorPos);
				if ( FrequencyCheck(GenConfig.freqPWM, bufferVal) )
				{
					GenConfig.freqSignal = bufferVal;
					snprintf(displayString, 12, "%d", GenConfig.freqSignal);
					num1Length = strlen(displayString);
					if ( num1CursorPos > num1Length ) num1CursorPos = num1Length;
					if ( GenConfig.isImmediateUpdate )
						UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.powerK, GenConfig.signalType);
		    }
		  break;
			case 2:
				bufferVal = getDeltaNumFromCursorPos(num2CursorPos);
			  num2Val += bufferVal;
				if ( num2Val > DeltaValMaxValue )
					num2Val = DeltaValMaxValue;
				snprintf(displayString, 12, "%d", num2Val);
				num2Length = strlen(displayString);
				if ( num2CursorPos > num2Length ) num2CursorPos = num2Length;
		  break;
	  };
	};
	
	//событие при нажатии кнопки 3
	if ( genEvents & EVENT_BUTTON3_CLICK )
	{
		cursorNumN--;
		if ( cursorNumN < 1 ) cursorNumN = 1;
	};
	
	//событие при нажатии кнопки 4
	if ( genEvents & EVENT_BUTTON4_CLICK )
	{
		cursorNumN++;
		if ( cursorNumN > 2 ) cursorNumN = 2;
	};
	
	return RESULT_OK;
}

uint8_t Menu4Draw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
		_copyStringToBuffer(&Line1BufferNew[0], "Power K", 4);
    //2 строка
		snprintf(displayString, 12, "%.2lf", GenConfig.powerK);
	  _copyStringToBuffer(&Line2BufferNew[0], displayString, 5);
		return RESULT_OK;
	};
	
	if ( ! (Display.displayOnOffControl & HD44780_FLAG_CURSORON) ) HD44780DisplaySetCursorVisible(&Display, 1);
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBuffer(&Line1Buffer[0], "Power K", 4);	
	//2 строка
	snprintf(displayString, 12, "%.2lf", GenConfig.powerK);
	_copyStringToBuffer(&Line2Buffer[0], displayString, 5);
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	//позиция курсора
	HD44780DisplayMoveCursor(&Display, 2, 6 + num1Length - num1CursorPos);
	return RESULT_OK;
}

uint8_t Menu4Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{
	//событие при удерживании нажатой кнопки валкодера и поворота по ч стрелке
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CW ) )
	{
		if ( num1CursorPos > 1 ) num1CursorPos--;
		if ( num1CursorPos == 3 ) num1CursorPos = 2;
		return RESULT_OK;
	};
	
	//событие при удерживании нажатой кнопки валкодера и поворота против ч стрелки
	if ( ( genEvents & EVENT_VALCODER_BUTTON_PRESSED ) && ( genEvents & EVENT_VALCODER_CCW ) )
	{
		if ( num1CursorPos < num1Length ) num1CursorPos++;
		if ( num1CursorPos == 3 ) num1CursorPos = 4;
		return RESULT_OK;
	};
	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		num1CursorPos = 1;
		snprintf(displayString, 12, "%d", GenConfig.freqSignal);
		num1Length = strlen(displayString);
		num2Val = 1;
		num2CursorPos = 1;
    num2Length = 1;
		cursorNumN = 1;
		menuTransDirection = 1; //вправо(-)
		
		Menu3Draw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		num1CursorPos = 1;
		snprintf(displayString, 12, "%.2lf", GenConfig.powerK);
		num1Length = strlen(displayString);
		num2Val = 1;
		num2CursorPos = 1;
    num2Length = 1;
		cursorNumN = 1;
		menuTransDirection = 2; //влево(+)
		
		Menu5Draw(0);
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		bufferVal = ( num1CursorPos > 3 ) ? getDeltaNumFromCursorPos(num1CursorPos-1) : getDeltaNumFromCursorPos(num1CursorPos);
		bufferValDouble = (double)bufferVal / 100;
		if ( ( GenConfig.powerK > bufferValDouble ) &&
         ( ( GenConfig.powerK - bufferValDouble ) >= MIN_POWER_K ) )
		{
			GenConfig.powerK -= bufferValDouble;
			snprintf(displayString, 12, "%.2lf", GenConfig.powerK);
			num1Length = strlen(displayString);
			if ( num1CursorPos > num1Length ) num1CursorPos = num1Length;
			if ( GenConfig.isImmediateUpdate )
				UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.powerK, GenConfig.signalType);
		};
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		bufferVal = ( num1CursorPos > 3 ) ? getDeltaNumFromCursorPos(num1CursorPos-1) : getDeltaNumFromCursorPos(num1CursorPos);
		bufferValDouble = (double)bufferVal / 100;
		if ( ( GenConfig.powerK + bufferValDouble ) <= MAX_POWER_K )
		{
			GenConfig.powerK += bufferValDouble;
			snprintf(displayString, 12, "%.2lf", GenConfig.powerK);
			num1Length = strlen(displayString);
			if ( GenConfig.isImmediateUpdate )
				UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.powerK, GenConfig.signalType);
		};
	};
	
	return RESULT_OK;
}

uint8_t Menu5Draw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
	  _copyStringToBuffer(&Line1BufferNew[0], "Signal  type", 2);	
	  //2 строка
	  switch ( GenConfig.signalType )
		{
			case 1:
				_copyStringToBuffer(&Line2BufferNew[0], "Sinus", 5);
		  break;
			
			case 2:
				_copyStringToBuffer(&Line2BufferNew[0], "Triangle", 4);
		  break;
	   };
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBuffer(&Line1Buffer[0], "Signal  type", 2);	
	//2 строка
	switch ( GenConfig.signalType )
	{
		case 1:
			_copyStringToBuffer(&Line2Buffer[0], "Sinus", 5);
		  MenuAnimSelectionDraw(&Line2Buffer[0], 5, 5, 10, GenMenu->MenuTargetDrawFPS, frameNum); 
		  Line2Buffer[15] = '<';
		break;
		
		case 2:
			_copyStringToBuffer(&Line2Buffer[0], "Triangle", 4);
		  MenuAnimSelectionDraw(&Line2Buffer[0], 4, 4, 12, GenMenu->MenuTargetDrawFPS, frameNum);
		break;
	};
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}


uint8_t Menu5Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		num1CursorPos = 1;
		snprintf(displayString, 12, "%.2lf", GenConfig.powerK);
		num1Length = strlen(displayString);
		num2Val = 1;
		num2CursorPos = 1;
    num2Length = 1;
		cursorNumN = 1;
		menuTransDirection = 1; //вправо(-)
		
		Menu4Draw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		menuTransDirection = 2; //влево(+)
		
		Menu6Draw(0);
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		if ( !--GenConfig.signalType ) GenConfig.signalType = 1;
		if ( GenConfig.isImmediateUpdate )
			UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.powerK, GenConfig.signalType);
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		if ( ++GenConfig.signalType > 2 ) GenConfig.signalType = 2;
		if ( GenConfig.isImmediateUpdate )
			UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.powerK, GenConfig.signalType);
	};
	
	return RESULT_OK;
}

uint8_t Menu6Draw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
		//1 строка
	  _copyStringToBuffer(&Line1BufferNew[0], "Signal  update", 1);	
	  //2 строка
	  if ( GenConfig.isImmediateUpdate )
	  {
		  _copyStringToBuffer(&Line2BufferNew[0], "Auto", 6);
	  } else
	  {
		  _copyStringToBuffer(&Line2BufferNew[0], "Manually", 4);
	  };
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//очистка
	_clearDisplayBuffers();
	//1 строка
	_copyStringToBuffer(&Line1Buffer[0], "Signal  update", 1);	
	//2 строка
	if ( GenConfig.isImmediateUpdate )
	{
		_copyStringToBuffer(&Line2Buffer[0], "Auto", 6);
		MenuAnimSelectionDraw(&Line2Buffer[0], 6, 6, 10, GenMenu->MenuTargetDrawFPS, frameNum);
	} else
	{
		_copyStringToBuffer(&Line2Buffer[0], "Manually", 4);
		MenuAnimSelectionDraw(&Line2Buffer[0], 4, 4, 12, GenMenu->MenuTargetDrawFPS, frameNum);
	};
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}


uint8_t Menu6Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		menuTransDirection = 1; //вправо(-)
		
		Menu5Draw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки 6
	if ( genEvents & EVENT_BUTTON6_CLICK )
  {
		menuTransDirection = 2; //влево(+)
		menu7_iteration = 0;
		
		Menu7Draw(0);
		MenuGoToNextItem(GenMenu);
	};
	
	//событие при вращении валкодера против ч стрелки
	if ( genEvents & EVENT_VALCODER_CCW )
	{
		GenConfig.isImmediateUpdate = 0;
	};
	
	//событие при вращении валкодера по ч стрелке
	if ( genEvents & EVENT_VALCODER_CW )
	{
		GenConfig.isImmediateUpdate = 1;
		UpdateSignal(GenConfig.freqPWM, GenConfig.freqSignal, GenConfig.powerK, GenConfig.signalType);
	};
	
	return RESULT_OK;
}

uint8_t Menu7Draw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
	  //1 строка
	  _copyStringToBuffer(&Line1BufferNew[0], "Save config?", 2);	
	  //2 строка
    _copyStringToBuffer(&Line2BufferNew[0], "Press valc button for save", 0);
		return RESULT_OK;
	};
	
	if ( Display.displayOnOffControl & HD44780_FLAG_CURSORON ) HD44780DisplaySetCursorVisible(&Display, 0);
	
	//очистка
	//_clearDisplayBuffers(); без очистки
	//1 строка
	_copyStringToBuffer(&Line1Buffer[0], "  Save config?  ", 0);	
	//2 строка
	if ( !( frameNum % 25 ) )
	{
		MenuTickerDraw(&Line2Buffer[0], "Press valcoder button for save", &menu7_iteration);
	};
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}


uint8_t Menu7Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents)
{	
	//событие при нажатии кнопки 5	
	if ( genEvents & EVENT_BUTTON5_CLICK )
	{
		menuTransDirection = 1; //вправо(-)
		
		Menu6Draw(0);
		MenuGoToPrevItem(GenMenu);
	};
	
	//событие при нажатии кнопки валкодера
	if ( genEvents & EVENT_VALCODER_BUTTON_CLICK )
	{
		bufferVal = 0;
		menuTransDirection = 2; //влево(+)
		
		MenuSaveDraw(0);
		MenuGoToChildItem(GenMenu);		
	};
	
	return RESULT_OK;
}

uint8_t MenuSaveDraw(const uint8_t frameNum)
{
	if ( frameNum == 0 )
	{
		_clearDisplayNewBuffers();
	  //1 строка
	  _copyStringToBuffer(&Line1BufferNew[0], "Config", 5);	
	  //2 строка
    _copyStringToBuffer(&Line2BufferNew[0], "......", 5);
		return RESULT_OK;
	};
	
	if ( !bufferVal )
	{
		//пауза 1 мс
		delayMS(1000);
		//очистка
	  _clearDisplayBuffers();
	  //1 строка
	  _copyStringToBuffer(&Line1Buffer[0], "Config", 5);	
	  //2 строка
		//сохранение
		if ( FlashWriteData( (uint8_t*)&GenConfig, sizeof(GenConfig) ) )
		{
			_copyStringToBuffer(&Line2Buffer[0], "saved!", 5);
		} else
		{
			_copyStringToBuffer(&Line2Buffer[0], "failed! :(", 3);
		};		
	};
	
	//в bufferVal инкрементируем количество кадров
	bufferVal++;
	//при bufferVal = ( 3 сек ) выходим
	if ( ( bufferVal * ( 1000 / GenMenu->MenuTargetDrawFPS ) ) > 3000 )
	{
		bufferVal = 0;
		menuTransDirection = 1; //вправо(-)
		
		Menu7Draw(0);
		MenuGoToParentItem(GenMenu);
	};	
	
	//выводим на дисплей
	HD44780DisplayWriteString(&Display, Line1Buffer, 1, 1);
	HD44780DisplayWriteString(&Display, Line2Buffer, 2, 1);
	return RESULT_OK;
}

