
#ifndef GEN_SYSTEM
#define GEN_SYSTEM

#include <stdint.h>

typedef enum 
{
	RESULT_ERROR = 0,
	RESULT_OK = 1,
	RESULT_FATAL_ERROR = 3,
	RESULT_IO_ERROR = 5
} ErrorCode;

void ErrorHandler(ErrorCode _code);
uint8_t GenSystemSet(void);

#endif
