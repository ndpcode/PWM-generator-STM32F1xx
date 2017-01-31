
#include "stm32f10x.h"
#include "settings.h"

void FlashWriteData(uint8_t *pwm_range_index, uint8_t *signal_range_index)
{
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR = flash_base_addr;
	FLASH->CR |= FLASH_CR_STRT;
	while( FLASH->SR & FLASH_SR_BSY );
	FLASH->CR &= ~FLASH_CR_PER;
	FLASH->CR |= FLASH_CR_PG;
	while( FLASH->SR & FLASH_SR_BSY );
	flash_addr1 = *pwm_range_index;
	while( FLASH->SR & FLASH_SR_BSY );
	flash_addr2 = *signal_range_index;
	while( FLASH->SR & FLASH_SR_BSY );
	FLASH->CR &= ~(FLASH_CR_PG);
	FLASH->CR |= FLASH_CR_LOCK;
}

void FlashReadData(uint8_t *pwm_range_index, uint8_t *signal_range_index)
{
	if ( ( flash_addr1 > 0 ) && ( flash_addr1 < 0xFF ) && ( flash_addr2 > 0 ) && ( flash_addr2 < 0xFF ) )
	{
		*pwm_range_index = flash_addr1;
		*signal_range_index = flash_addr2;		
	} else
	{
		*pwm_range_index = 6;
		*signal_range_index = 1;
	};
	
}
