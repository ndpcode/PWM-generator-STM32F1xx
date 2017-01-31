
#ifndef GEN_SYSTEM
#define GEN_SYSTEM

#include <stdint.h>

#define RESULT_OK 1
#define RESULT_FATAL_ERROR 2
#define RESULT_IO_ERROR 3

void Error_Handler(uint8_t Error_Code);
uint8_t gen_system_set(void);

#endif
