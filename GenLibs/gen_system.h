
#ifndef GEN_SYSTEM
#define GEN_SYSTEM

#include <stdint.h>

typedef enum 
{
	RESULT_ERROR = 0,
	RESULT_OK = 1,
	RESULT_FATAL_ERROR,
	RESULT_IO_ERROR
} ErrorCode;

void ErrorHandler(ErrorCode _code);
uint8_t GenSystemSet(void);

#endif
