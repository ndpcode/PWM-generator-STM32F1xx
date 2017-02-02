
#include "settings.h"
#include "gen_system.h"

void ErrorHandler(ErrorCode _code)
{
	switch ( _code )
	{
		case RESULT_ERROR:
		break;
		case RESULT_OK:
		break;
		
		case RESULT_FATAL_ERROR:
			//CloseAll();
		  LED_GREEN_ON;
		  LED_BLUE_ON;
		  LED_RED_ON;
		  //ждать до аппаратного сброса
      while (1);		
		break;
		
		case RESULT_IO_ERROR:
		break;
	}
}

uint8_t GenSystemSet(void)
{
	return RESULT_OK;
}
