
#ifndef HD44780_DISPLAY_H
#define HD44780_DISPLAY_H

#include "stm32f1xx_hd44780_driver.h"

#define HD44780_CMD_CLEARDISPLAY 0x01
#define HD44780_CMD_RETURNHOME 0x02
#define HD44780_CMD_ENTRYMODESET 0x04
#define HD44780_CMD_DISPLAYCONTROL 0x08
#define HD44780_CMD_CURSORSHIFT 0x10
#define HD44780_CMD_FUNCTIONSET 0x20
#define HD44780_CMD_SETCGRAMADDR 0x40
#define HD44780_CMD_SETDDRAMADDR 0x80

// flags for display entry mode
#define HD44780_FLAG_ENTRYRIGHT 0x02
#define HD44780_FLAG_ENTRYLEFT 0x00
#define HD44780_FLAG_ENTRYSHIFTON 0x01
#define HD44780_FLAG_ENTRYSHIFTOFF 0x00

// flags for display on/off control
#define HD44780_FLAG_DISPLAYON 0x04
#define HD44780_FLAG_DISPLAYOFF 0x00
#define HD44780_FLAG_CURSORON 0x02
#define HD44780_FLAG_CURSOROFF 0x00
#define HD44780_FLAG_BLINKON 0x01
#define HD44780_FLAG_BLINKOFF 0x00

// flags for display/cursor shift
#define HD44780_FLAG_DISPLAYMOVE 0x08
#define HD44780_FLAG_CURSORMOVE 0x00
#define HD44780_FLAG_MOVERIGHT 0x04
#define HD44780_FLAG_MOVELEFT 0x00

// flags for function set
#define HD44780_FLAG_8BITMODE 0x10
#define HD44780_FLAG_4BITMODE 0x00
#define HD44780_FLAG_2LINE 0x08
#define HD44780_FLAG_1LINE 0x00
#define HD44780_FLAG_5x10DOTS 0x04
#define HD44780_FLAG_5x8DOTS 0x00

enum
{
	HD44780_DISPLAY_OK = 17,
	HD44780_DISPLAY_ERROR = 19,
	HD44780_DISPLAY_CONFIG_ERROR = 21
};

typedef enum
{
	HD44780_DISPLAY_COMMAND = 0,
	HD44780_DISPLAY_DATA		
} HD44780_DISPLAY_BYTE_TYPE;

typedef struct
{
	HD44780_PORT_STRUCT *portStruct;
	void (*delayUSFunc)(unsigned short);
	unsigned char displayEntryMode;
  unsigned char displayOnOffControl;
	unsigned char displayFunctionSet;	
} HD44780_DISPLAY_STRUCT;

unsigned char HD44780DisplayInit(HD44780_DISPLAY_STRUCT *displayStruct);
unsigned char HD44780DisplayClear(HD44780_DISPLAY_STRUCT *displayStruct);
unsigned char HD44780DisplaySendByte(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char displayData,
																		 HD44780_DISPLAY_BYTE_TYPE isData);
unsigned char HD44780DisplayMoveCursor(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char displayRow,
	                                     unsigned char displayColumn);
unsigned char HD44780DisplayWriteChar(HD44780_DISPLAY_STRUCT *displayStruct, char displayChar,
																			unsigned char displayRow, unsigned char displayColumn);
unsigned char HD44780DisplayWriteString(HD44780_DISPLAY_STRUCT *displayStruct, const char *displayString,
																			  unsigned char displayRow, unsigned char displayColumn);
unsigned char HD44780DisplayWriteInt32(HD44780_DISPLAY_STRUCT *displayStruct, int displayInt,
                                       unsigned char displayRow, unsigned char displayColumn);
unsigned char HD44780DisplayWriteDouble(HD44780_DISPLAY_STRUCT *displayStruct, double displayDouble, char doubleAccuracy,
                                        unsigned char displayRow, unsigned char displayColumn);
unsigned char HD44780DisplayDisplayOnOff(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char isOn);
unsigned char HD44780DisplaySetCursorVisible(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char isCursor);
unsigned char HD44780DisplaySetBlinkVisible(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char isBlink);
unsigned char HD44780DisplaySetScrollDirection(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char isLeftToRight);
unsigned char HD44780DisplaySetAutoscroll(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char isAutoscroll);


#endif
