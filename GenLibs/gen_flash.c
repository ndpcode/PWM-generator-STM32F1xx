
#include "settings.h"
#include "gen_flash.h"

uint8_t FlashAccessEnable(void)
{
	//вкл. доступ к flash
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
	return 1;
}

uint8_t FlashWriteData(uint8_t *byte_array, uint8_t size)
{
	uint8_t i = 0;
	
	// size - не 0, кратный 2
	if ( size == 0 ) return 0;
	if ( ( size % 2 ) != 0 ) return 0;
	//пустой указатель
	if ( !byte_array ) return 0;
	
	//ждем
	while( FLASH->SR & FLASH_SR_BSY );
	//если установлен флаг завершения операции, сброс
	if (FLASH->SR & FLASH_SR_EOP) FLASH->SR |= FLASH_SR_EOP;
	
	//очистка страницы с адресом flash_base_add
	//режим - очистка страницы
	FLASH->CR |= FLASH_CR_PER;
	//адрес старницы
	FLASH->AR = flash_base_addr;
	//старт очистки
	FLASH->CR |= FLASH_CR_STRT;
	//ждем флага и сбрасываем
	while ( !( FLASH->SR & FLASH_SR_EOP ) );
	FLASH->SR |= FLASH_SR_EOP;
	//выкл режим
	FLASH->CR &= ~FLASH_CR_PER;
	
	//ждем
	while( FLASH->SR & FLASH_SR_BSY );
	//если установлен флаг завершения операции, сброс
	if (FLASH->SR & FLASH_SR_EOP) FLASH->SR |= FLASH_SR_EOP;
	
	//режим - запись во flash	
	FLASH->CR |= FLASH_CR_PG;
	
	//запись
	for ( i = 0; i < size; i += 2 )
	{
		//пишем в память
		*(volatile uint16_t*)(flash_base_addr + i) = (uint16_t)( byte_array[i] + ( byte_array[i+1] << 8 ) );
		//ждем флага и сбрасываем
		while ( !( FLASH->SR & FLASH_SR_EOP ) );
		FLASH->SR |= FLASH_SR_EOP;
	};
	
	//выкл режим
	FLASH->CR &= ~(FLASH_CR_PG);
	
	return 1;
}

uint8_t FlashReadData(uint8_t *byte_array, uint8_t size)
{
	uint8_t i = 0;
	// size - не 0, кратный 2
	if ( size == 0 ) return 0;
	if ( ( size % 2 ) != 0 ) return 0;
	//чтение flash в массив
	for ( i = 0; i < size; i += 2 )
	{
		byte_array[i] = ( *(volatile uint16_t*)(flash_base_addr+i) ) & 0x00FF;
		byte_array[i+1] = ( ( *(volatile uint16_t*)(flash_base_addr+i) ) & 0xFF00 ) >> 8;		
	};
	return 1;	
}
