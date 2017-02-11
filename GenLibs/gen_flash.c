
#include "settings.h"
#include "gen_flash.h"

uint8_t FlashAccessEnable(void)
{
	//���. ������ � flash
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
	return 1;
}

uint8_t FlashWriteData(uint8_t *byte_array, uint8_t size)
{
	uint8_t i = 0;
	
	// size - �� 0, ������� 2
	if ( size == 0 ) return 0;
	if ( ( size % 2 ) != 0 ) return 0;
	//������ ���������
	if ( !byte_array ) return 0;
	
	//����
	while( FLASH->SR & FLASH_SR_BSY );
	//���� ���������� ���� ���������� ��������, �����
	if (FLASH->SR & FLASH_SR_EOP) FLASH->SR |= FLASH_SR_EOP;
	
	//������� �������� � ������� flash_base_add
	//����� - ������� ��������
	FLASH->CR |= FLASH_CR_PER;
	//����� ��������
	FLASH->AR = flash_base_addr;
	//����� �������
	FLASH->CR |= FLASH_CR_STRT;
	//���� ����� � ����������
	while ( !( FLASH->SR & FLASH_SR_EOP ) );
	FLASH->SR |= FLASH_SR_EOP;
	//���� �����
	FLASH->CR &= ~FLASH_CR_PER;
	
	//����
	while( FLASH->SR & FLASH_SR_BSY );
	//���� ���������� ���� ���������� ��������, �����
	if (FLASH->SR & FLASH_SR_EOP) FLASH->SR |= FLASH_SR_EOP;
	
	//����� - ������ �� flash	
	FLASH->CR |= FLASH_CR_PG;
	
	//������
	for ( i = 0; i < size; i += 2 )
	{
		//����� � ������
		*(volatile uint16_t*)(flash_base_addr + i) = (uint16_t)( byte_array[i] + ( byte_array[i+1] << 8 ) );
		//���� ����� � ����������
		while ( !( FLASH->SR & FLASH_SR_EOP ) );
		FLASH->SR |= FLASH_SR_EOP;
	};
	
	//���� �����
	FLASH->CR &= ~(FLASH_CR_PG);
	
	return 1;
}

uint8_t FlashReadData(uint8_t *byte_array, uint8_t size)
{
	uint8_t i = 0;
	// size - �� 0, ������� 2
	if ( size == 0 ) return 0;
	if ( ( size % 2 ) != 0 ) return 0;
	//������ flash � ������
	for ( i = 0; i < size; i += 2 )
	{
		byte_array[i] = ( *(volatile uint16_t*)(flash_base_addr+i) ) & 0x00FF;
		byte_array[i+1] = ( ( *(volatile uint16_t*)(flash_base_addr+i) ) & 0xFF00 ) >> 8;		
	};
	return 1;	
}
