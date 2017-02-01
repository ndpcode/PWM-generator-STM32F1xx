
#include "settings.h"
#include "gen_system.h"
#include "gen_flash.h"
#include "gen_ports.h"
#include "gen_timers.h"
#include "gen_controls.h"
#include "bsc_stm32_delay.h"
#include <limits.h>

uint8_t InitDefaults(void);

int main(void)
{
	InitDefaults();
	//разрешаем использование функций delay библиотеки bsc_stm32_delay
	delayEnable();
	
	//старт МК, настройка тактовых генераторов и часов
	GenSystemSet();
	//настройка портов
	GenInitPorts();
	//разрешаем доступ к flash MCU
	FlashAccessEnable();
	//чтение настроек из flash
	
	//настройка таймеров
	gen_init_timers();
	
	while(1)
	{
	}
}

uint8_t InitDefaults(void)
{
	return RESULT_OK;
}
