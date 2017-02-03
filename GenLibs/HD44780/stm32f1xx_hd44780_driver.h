
#ifndef STM32F1XX_HD44780_DRIVER
#define STM32F1XX_HD44780_DRIVER

#if !defined (STM32F10X_LD) && !defined (STM32F10X_LD_VL) && !defined (STM32F10X_MD) && !defined (STM32F10X_MD_VL) && !defined (STM32F10X_HD) && !defined (STM32F10X_HD_VL) && !defined (STM32F10X_XL) && !defined (STM32F10X_CL) 
  /* #define STM32F10X_LD */     /*!< STM32F10X_LD: STM32 Low density devices */
  /* #define STM32F10X_LD_VL */  /*!< STM32F10X_LD_VL: STM32 Low density Value Line devices */  
   #define STM32F10X_MD
  /* #define STM32F10X_MD_VL */  /*!< STM32F10X_MD_VL: STM32 Medium density Value Line devices */  
  /* #define STM32F10X_HD */     /*!< STM32F10X_HD: STM32 High density devices */
  /* #define STM32F10X_HD_VL */  /*!< STM32F10X_HD_VL: STM32 High density value line devices */  
  /* #define STM32F10X_XL */     /*!< STM32F10X_XL: STM32 XL-density devices */
  /* #define STM32F10X_CL */     /*!< STM32F10X_CL: STM32 Connectivity line devices */
#endif

#ifndef __STM32F10x_H 
#include <stm32f10x.h>
#endif

enum
{
	HD44780_DRIVER_ERROR = 11,
	HD44780_DRIVER_OK = 15
};

typedef enum
{
	HD44780_DRIVER_ENABLE = 0,
	HD44780_DRIVER_REG_SELECT,
	HD44780_DRIVER_READ_WRITE,
	HD44780_DRIVER_DATA_0,
	HD44780_DRIVER_DATA_1,
	HD44780_DRIVER_DATA_2,
	HD44780_DRIVER_DATA_3,
	HD44780_DRIVER_DATA_4,
	HD44780_DRIVER_DATA_5,
	HD44780_DRIVER_DATA_6,
	HD44780_DRIVER_DATA_7	
} HD44780_DISPLAY_PIN_NAME;

typedef enum
{
	HD44780_PIN_DIRECTION_INPUT = 0,
	HD44780_PIN_DIRECTION_OUTPUT,
	HD44780_PIN_DIRECTION_FLOATING	
} HD44780_PIN_DIRECTION;

typedef struct
{
	GPIO_TypeDef *portAddress;
	unsigned char pinIndex;	
} HD44780_PIN_STRUCT;

typedef struct
{
	HD44780_PIN_STRUCT DisplayPort[11];
} HD44780_PORT_STRUCT;

unsigned char HD44780DriverSetPin(HD44780_PORT_STRUCT *_portStruct,
	                                HD44780_DISPLAY_PIN_NAME _pinName,
                                  HD44780_PIN_DIRECTION _pinDirection);
unsigned char HD44780DriverWritePin(HD44780_PORT_STRUCT *_portStruct,
                                    HD44780_DISPLAY_PIN_NAME _pinName,
                                    unsigned char _logicLevel);
unsigned char HD44780DriverReadPin(HD44780_PORT_STRUCT *_portStruct,
	                                 HD44780_DISPLAY_PIN_NAME _pinName);

#endif
