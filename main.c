
#include "stm32f10x.h"
#include "settings.h"
#include "gen_system.h"
#include "gen_ports.h"
#include "gen_timers.h"
#include "gen_controls.h"
#include "bsc_stm32_delay.h"
#include <limits.h>

uint8_t InitDefaults(void);

int main(void)
{
	InitDefaults();
	delayEnable();
	
	//init
	gen_system_set();
	gen_init_ports();
	gen_init_timers();
	
	while(1)
	{
	}
}

uint8_t InitDefaults(void)
{
	return RESULT_OK;
}
