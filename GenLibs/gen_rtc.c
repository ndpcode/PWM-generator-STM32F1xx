
#include "settings.h"
#include "gen_rtc.h"

uint8_t GenInitRTC(void)
{
	//power and backup registers access enable
	RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
	//Enable access to RTC
	PWR->CR |= PWR_CR_DBP;
	//Backup domain reset
	RCC->BDCR |= RCC_BDCR_BDRST;
	RCC->BDCR &= ~RCC_BDCR_BDRST;
	//LSE ON
	RCC->BDCR |= RCC_BDCR_LSEON;
	//LSI ON
	RCC->CSR |= RCC_CSR_LSION;
	//LSI as cource for RTC
	RCC->BDCR |= RCC_BDCR_RTCSEL_LSI;	
	//RTC enable
	RCC->BDCR |= RCC_BDCR_RTCEN;
	
	//Wait the registers to be synchronised
	RTC->CRL &= ~RTC_CRL_RSF;
	while ( !(RTC->CRL & RTC_CRL_RSF) );	
	//Poll RTOFF, wait until its value goes to 1
	while ( !(RTC->CRL & RTC_CRL_RTOFF) );
	//Set the CNF bit to enter configuration mode
	RTC->CRL |= RTC_CRL_CNF;
	//Setup RTC
	RTC->PRLH  = ((32 - 1) >> 16) & 0x00FF;
  RTC->PRLL  = ((32 - 1)      ) & 0xFFFF;
	RTC->CNTH = 0;
	RTC->CNTL = 0;
	//Clear the CNF bit to exit configuration mode
	RTC->CRL &= ~RTC_CRL_CNF;
	//Poll RTOFF, wait until its value goes to 1
	while ( !(RTC->CRL & RTC_CRL_RTOFF) );
	//Disable access to RTC
	PWR->CR &= ~PWR_CR_DBP;	
	return 1;
}
