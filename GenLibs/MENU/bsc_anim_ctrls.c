
#include "settings.h"
#include "bsc_anim_ctrls.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//*******************************************************************************************************************//
//Вспомогательные функции//
//*******************************************************************************************************************//

uint8_t AnimVarNumberCheckConfig(ANIM_VAR_NUMBER *varNumber)
{
	if ( !varNumber ) return 0;
	if ( !varNumber->displayStruct ) return 0;
	if ( ( !varNumber->row ) || ( !varNumber->column ) ) return 0;
	if ( !varNumber->targetNumber ) return 0;
	if ( !varNumber->cursorPosition ) return 0;
	if ( !varNumber->incSize ) return 0;
	return 1;
}

uint32_t getDeltaNumFromCursorPos(uint8_t _cursorPos)
{
	uint32_t result = 1;
	if ( !_cursorPos ) return 0;
	while ( --_cursorPos )
		result *= 10;
	return result;		
}

uint8_t getTargetNumLength(ANIM_VAR_NUMBER *varNumber)
{
	//**********************************//
	//либо алгоритм с преобразованием в double
	//либо с условиями
	//**********************************//
	uint8_t result;
	char stringBuffer[16];
	
	//************* алгоритм с преобразованием в double *************//
	char _a[6] = "%.";
	char _b[2] = "  ";
	char _c[2] = "lf";
	uint32_t _mult;
	
	_mult = 10 * getDeltaNumFromCursorPos(varNumber->doublePointPosition);
	if ( !_mult ) _mult = 1;
	
	//varNumber->doublePointPosition to string _b
	snprintf(_b, 3, "%d", varNumber->doublePointPosition);
	strncat(_a, _b, strlen(_b));
	strncat(_a, _c, 2);
	snprintf(stringBuffer, 17, _a, (double)(*varNumber->targetNumber) / _mult );
	
	//находим длину числа
	result = strlen(stringBuffer);
	
	//************* алгоритм с условиями *************//
	/*
	//преобоазуем в строку число по модулю
	snprintf(stringBuffer, 16, "%d", abs(*varNumber->targetNumber));
	//находим длину числа
	result = strlen(stringBuffer);
	//сравниваем с необходимым количеством точек
	if ( result <= varNumber->doublePointPosition )
		result += varNumber->doublePointPosition - result + 1;
	//учитываем точку
	if ( varNumber->doublePointPosition )
		result++;
	//учитываем знак числа
	if ( (*varNumber->targetNumber) < 0 )
		result++;
	*/
	
	return result;
}

void AnimVarNumberCheckCursor(ANIM_VAR_NUMBER *varNumber)
{
	uint8_t numLength;

	//находим длину числа
	numLength = getTargetNumLength(varNumber);
	//знак "-" - уменьшаем длину на 1
	if ( (*varNumber->targetNumber) < 0 ) numLength--;
	
	if ( varNumber->cursorPosition > numLength ) varNumber->cursorPosition = numLength;
	
	if ( ( varNumber->doublePointPosition ) && ( varNumber->cursorPosition > (varNumber->doublePointPosition + 1) ) ) 
	{
		varNumber->incSize = getDeltaNumFromCursorPos(varNumber->cursorPosition - 1);
	} else
	{
		varNumber->incSize = getDeltaNumFromCursorPos(varNumber->cursorPosition);
	}
}

//*******************************************************************************************************************//
//Интерфейс изменяемого числа с курсором//
//*******************************************************************************************************************//

ANIM_VAR_NUMBER *CreateAnimVarNumber(HD44780_DISPLAY_STRUCT *_displayStruct, uint8_t _row, uint8_t _column,
                                     int32_t *_targetNumber, uint8_t _doublePointPosition, int32_t *_externIncSize)
{
	ANIM_VAR_NUMBER *aVN = 0;
	if ( !_displayStruct ) return 0;
	if ( ( !_row ) || ( !_column ) ) return 0;
	if ( !_targetNumber ) return 0;
	
	aVN = (ANIM_VAR_NUMBER*)malloc(sizeof(ANIM_VAR_NUMBER));
	if ( !aVN ) return 0;
	aVN->displayStruct = _displayStruct;
	aVN->row = _row;
	aVN->column = _column;
	aVN->targetNumber = _targetNumber;
	aVN->doublePointPosition = _doublePointPosition;
	aVN->cursorPosition = 1;
	aVN->incSize = 1;
	aVN->externIncSize = _externIncSize;
	aVN->varEnable = 0;
	
	return aVN;
}

uint8_t DestroyAnimVarNumber(ANIM_VAR_NUMBER **varNumber)
{
	if ( !varNumber ) return 0;
	if ( !*varNumber ) return 0;
	
	free(*varNumber);
	*varNumber = 0;
	return 1;
}

uint8_t AnimVarNumberDraw(ANIM_VAR_NUMBER *varNumber)
{	
	char displayBuffer[16];	
	char _a[6] = "%.";
	char _b[2] = "  ";
	char _c[2] = "lf";
	uint32_t _mult;
	uint8_t numLength;
	
	if ( !AnimVarNumberCheckConfig(varNumber) ) return 0;
	
	//проверка включения курсора
	if ( ! (varNumber->displayStruct->displayOnOffControl & HD44780_FLAG_CURSORON) )
		HD44780DisplaySetCursorVisible(varNumber->displayStruct, 1);
	
	_mult = 10 * getDeltaNumFromCursorPos(varNumber->doublePointPosition);
	if ( !_mult ) _mult = 1;
	
	//varNumber->doublePointPosition to string _b
	snprintf(_b, 3, "%d", varNumber->doublePointPosition);
	strncat(_a, _b, strlen(_b));
	strncat(_a, _c, 2);
	snprintf(displayBuffer, 17, _a, (double)(*varNumber->targetNumber) / _mult );
	
	//находим длину числа
	numLength = strlen(displayBuffer);

  //обрезать строку, если не помещается
  if ( varNumber->column < numLength )
	{
		strncpy(displayBuffer, displayBuffer + (numLength - varNumber->column), varNumber->column);
		numLength = varNumber->column;
	};		
	
	//выводим строку
	HD44780DisplayWriteString(varNumber->displayStruct, displayBuffer, varNumber->row, varNumber->column - numLength + 1);
	
	return 1;
}

uint8_t AnimVarNumberInc(ANIM_VAR_NUMBER *varNumber)
{
	if ( !AnimVarNumberCheckConfig(varNumber) ) return 0;
	if ( !varNumber->varEnable ) return 0;
	
	if ( !varNumber->externIncSize )
	{
		if ( (int32_t)(*varNumber->targetNumber) <= (int32_t)( 2147483647 - varNumber->incSize ) )
			(*varNumber->targetNumber) += varNumber->incSize;			
	} else
	{
		if ( (int32_t)(*varNumber->targetNumber) <= (int32_t)( 2147483647 - (*varNumber->externIncSize) * varNumber->incSize ) )
			(*varNumber->targetNumber) += (*varNumber->externIncSize) * varNumber->incSize;
	};
	
	AnimVarNumberCheckCursor(varNumber);
	
	return 1;
}

uint8_t AnimVarNumberDec(ANIM_VAR_NUMBER *varNumber)
{
	if ( !AnimVarNumberCheckConfig(varNumber) ) return 0;
	if ( !varNumber->varEnable ) return 0;
	
	if ( !varNumber->externIncSize )
	{
		if ( (int32_t)*(varNumber->targetNumber) >= (int32_t)( (int32_t)-2147483647 + varNumber->incSize ) )
			(*varNumber->targetNumber) -= varNumber->incSize;			
	} else
	{
		if ( (int32_t)*(varNumber->targetNumber) >= (int32_t)( (int32_t)-2147483647 + (*varNumber->externIncSize) * varNumber->incSize ) )
			(*varNumber->targetNumber) -= (*varNumber->externIncSize) * varNumber->incSize;
	};
	
  AnimVarNumberCheckCursor(varNumber);
	
	return 1;
}

uint8_t AnimVarNumberCursorMoveLeft(ANIM_VAR_NUMBER *varNumber)
{
	uint8_t numLength;
	if ( !AnimVarNumberCheckConfig(varNumber) ) return 0;
	if ( !varNumber->varEnable ) return 0;

	//находим длину числа
	numLength = getTargetNumLength(varNumber);
	//знак "-" - уменьшаем длину на 1
	if ( (*varNumber->targetNumber) < 0 ) numLength--;
	
	if ( varNumber->cursorPosition < 0xFF ) varNumber->cursorPosition++;
	if ( ( varNumber->doublePointPosition ) &&
		   ( varNumber->cursorPosition == (varNumber->doublePointPosition + 1) ) )
	{
		varNumber->cursorPosition++;
	};
	
	if ( varNumber->cursorPosition > numLength ) varNumber->cursorPosition = numLength;
	
	if ( ( varNumber->doublePointPosition ) && ( varNumber->cursorPosition > (varNumber->doublePointPosition + 1) ) ) 
	{
		varNumber->incSize = getDeltaNumFromCursorPos(varNumber->cursorPosition - 1);
	} else
	{
		varNumber->incSize = getDeltaNumFromCursorPos(varNumber->cursorPosition);
	}
	
	return 1;
}

uint8_t AnimVarNumberCursorMoveRight(ANIM_VAR_NUMBER *varNumber)
{
	if ( !AnimVarNumberCheckConfig(varNumber) ) return 0;
	if ( !varNumber->varEnable ) return 0;
	
	if ( varNumber->cursorPosition > 1 ) varNumber->cursorPosition--;
	if ( ( varNumber->doublePointPosition ) &&
		   ( varNumber->cursorPosition == (varNumber->doublePointPosition + 1) ) )
	{
		varNumber->cursorPosition--;
	};
	
	if ( ( varNumber->doublePointPosition ) && ( varNumber->cursorPosition > (varNumber->doublePointPosition + 1) ) ) 
	{
		varNumber->incSize = getDeltaNumFromCursorPos(varNumber->cursorPosition - 1);
	} else
	{
		varNumber->incSize = getDeltaNumFromCursorPos(varNumber->cursorPosition);
	}
	
	return 1;
}

uint8_t AnimVarNumberEnable(ANIM_VAR_NUMBER *varNumber, uint8_t isEnable)
{
	if ( !AnimVarNumberCheckConfig(varNumber) ) return 0;
	if ( ( isEnable != 0 ) && ( isEnable != 1 ) ) return 0;
	
	varNumber->varEnable = isEnable;
	
	return 1;
}

uint8_t AnimVarNumberDrawCursor(ANIM_VAR_NUMBER *varNumber)
{
	if ( !AnimVarNumberCheckConfig(varNumber) ) return 0;
	if ( !varNumber->varEnable ) return 0;
	
	//рисуем курсор
	HD44780DisplayMoveCursor(varNumber->displayStruct, varNumber->row, varNumber->column - varNumber->cursorPosition + 1);
	
	return 1;
}


//*******************************************************************************************************************//
//Функции вывода анимации дисплея//
//*******************************************************************************************************************//

uint8_t MenuAnimSelectionDraw(char *_buffer, uint8_t _selLength, uint8_t leftBorder,
	                         uint8_t rightBorder, uint8_t lineLength, uint8_t frameAll, uint8_t frameNum)
{
	uint8_t i = 0;
	if ( !_buffer ) return 0;
	if ( !_selLength ) return 0;
	if ( !lineLength ) return 0;
	if ( !frameAll ) return 0;
	if ( !frameNum ) return 0;	
	if ( ( leftBorder >= _selLength ) && ( ( rightBorder + _selLength - 1 ) <= lineLength ) )
		for ( i = 0; i < _selLength; i++ )
	  {
		  if ( frameNum >= ( (double)i * ( frameAll / _selLength ) ) )
		  {
			  _buffer[leftBorder - _selLength + i] = 0xC9;
	      _buffer[rightBorder + _selLength - i - 2] = 0xC8;
		  }
	  };
	return 1;
}

//отрисовка плавающей (вправо - влево) строки, если целиком не помещается в поле длиной lineLength
uint8_t MenuFloatingStrDraw(char *_buffer, char *floatText, uint8_t startPos, uint8_t lineLength, int16_t *iteration)
{
	uint8_t i = 0,
	        floatLength = 0,
	        _shift = 0;
	if ( !_buffer ) return 0;
	if ( !floatText ) return 0;
	if ( !startPos ) return 0;
	if ( !lineLength ) return 0;
	if ( !iteration ) return 0;	
	if ( ( strlen(_buffer) - startPos + 1 ) < lineLength ) return 0;
	
	floatLength = strlen(floatText);	
	if ( lineLength < floatLength )
	{
		if ( ( abs(*iteration) + lineLength ) <= floatLength )
	  {
		  for ( i = 0; i < lineLength; i++ )
		  {
			  _buffer[startPos - 1 + i] = floatText[i + abs(*iteration)];		
		  }
	  };
	  (*iteration)++;
    if ( ( abs(*iteration) + lineLength ) > floatLength ) *iteration = -(*iteration - 2);
  } else
	{
		_shift = ( lineLength - ( startPos - 1 ) - floatLength ) / 2 + ( startPos - 1 );
		for ( i = 0; i < floatLength; i++ )
		{
			_buffer[_shift + i] = floatText[i];		
		}
	};

  return 1;	
}

//отрисовка бегущей (справа налево) строки, если целиком не помещается в поле длиной lineLength
uint8_t MenuTickerStrDraw(char *_buffer, char *tickerText, uint8_t startPos, uint8_t lineLength, uint16_t *iteration)
{
	uint8_t i = 0,
	        tickerLength = 0,
	        _shift = 0;
	if ( !_buffer ) return 0;
	if ( !tickerText ) return 0;
	if ( !startPos ) return 0;
	if ( !lineLength ) return 0;
	if ( !iteration ) return 0;	
	if ( ( strlen(_buffer) - startPos + 1 ) < lineLength ) return 0;
	
	tickerLength = strlen(tickerText);
	if ( lineLength < tickerLength )
	{
		for ( i = 0; i < lineLength; i++ )
		{
			_buffer[startPos - 1 + i] = tickerText[ ( i + (*iteration) ) % tickerLength ];		
		}
		if ( (*iteration) >= tickerLength ) (*iteration) = 0;
	  (*iteration)++;
  } else
	{
		_shift = ( lineLength - ( startPos - 1 ) - tickerLength ) / 2 + ( startPos - 1 );
		for ( i = 0; i < tickerLength; i++ )
		{
			_buffer[_shift + i] = tickerText[i];		
		}
	};

  return 1;	
}
