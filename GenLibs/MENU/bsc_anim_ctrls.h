
#ifndef BSC_ANIMATED_CONTROLS
#define BSC_ANIMATED_CONTROLS

#include "hd44780display.h"

//интерфейс поля настройки числа с курсором
typedef struct
{
	HD44780_DISPLAY_STRUCT *displayStruct;
	uint8_t row, column;
	int32_t *targetNumber;
	uint8_t doublePointPosition;
	int32_t incSize;
	uint8_t cursorPosition;
	int32_t *externIncSize;
	uint8_t varEnable;
} ANIM_VAR_NUMBER;

//функции поля настройки числа с курсором
ANIM_VAR_NUMBER *CreateAnimVarNumber(HD44780_DISPLAY_STRUCT *_displayStruct, uint8_t _row, uint8_t _column,
                                     int32_t *_targetNumber, uint8_t _doublePointPosition, int32_t *_externIncSize);
uint8_t DestroyAnimVarNumber(ANIM_VAR_NUMBER **varNumber);
uint8_t AnimVarNumberDraw(ANIM_VAR_NUMBER *varNumber);
uint8_t AnimVarNumberInc(ANIM_VAR_NUMBER *varNumber);
uint8_t AnimVarNumberDec(ANIM_VAR_NUMBER *varNumber);
uint8_t AnimVarNumberCursorMoveLeft(ANIM_VAR_NUMBER *varNumber);
uint8_t AnimVarNumberCursorMoveRight(ANIM_VAR_NUMBER *varNumber);
uint8_t AnimVarNumberEnable(ANIM_VAR_NUMBER *varNumber, uint8_t isEnable);
uint8_t AnimVarNumberDrawCursor(ANIM_VAR_NUMBER *varNumber);

//отрисовка анимированного поля с текстом внутри ">>>>текст<<<<"
uint8_t MenuAnimSelectionDraw(char *_buffer, uint8_t _selLength, uint8_t leftBorder,
	                         uint8_t rightBorder, uint8_t lineLength, uint8_t frameAll, uint8_t frameNum);

//отрисовка плавающей (вправо - влево) строки, если целиком не помещается в поле длиной lineLength
uint8_t MenuFloatingStrDraw(char *_buffer, char *floatText, uint8_t startPos, uint8_t lineLength, int16_t *iteration);
//отрисовка бегущей (справа налево) строки, если целиком не помещается в поле длиной lineLength
uint8_t MenuTickerStrDraw(char *_buffer, char *tickerText, uint8_t startPos, uint8_t lineLength, uint16_t *iteration);

#endif
