
#ifndef GEN_FLASH
#define GEN_FLASH

#include <stdint.h>

uint8_t FlashAccessEnable(void);
uint8_t FlashWriteData(uint8_t *byte_array, uint8_t size);
uint8_t FlashReadData(uint8_t *byte_array, uint8_t size);

#endif
