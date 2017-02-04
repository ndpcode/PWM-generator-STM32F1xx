
#ifndef HD44780_DISPLAY_H
#define HD44780_DISPLAY_H

#include "stm32f1xx_hd44780_driver.h"

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
} HD44780_DISPLAY_STRUCT;

unsigned char HD44780DisplayInit(HD44780_DISPLAY_STRUCT *displayStruct);
unsigned char HD44780DisplayClear(HD44780_DISPLAY_STRUCT *displayStruct);
unsigned char HD44780DisplaySendByte(HD44780_DISPLAY_STRUCT *displayStruct,
                                                 unsigned char displayData,
																				  HD44780_DISPLAY_BYTE_TYPE isData);

#endif
