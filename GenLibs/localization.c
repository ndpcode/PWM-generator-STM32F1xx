
#include "localization.h"
#include <stdlib.h>
#include <string.h>

char **locLanguageData;

void locLanguageDataClear(void)
{
	uint8_t i;
	if ( locLanguageData )
	{
		for ( i = 0; i < L_NUM; i++ )
		{
			if ( locLanguageData[i] )
			{
				free(locLanguageData[i]);
				locLanguageData[i] = 0;
			};			
		};
    free(locLanguageData);
		locLanguageData = 0;
	};
}

void locAddStr(uint8_t *index, char *str)
{
	locLanguageData[*index] = (char*)malloc( sizeof(char) * ( strlen(str) + 1) );
	strcpy(locLanguageData[*index], str);	
	(*index)++;
}

uint8_t GenChangeLocalization(uint8_t locId)
{
	uint8_t i = 0;
	locLanguageDataClear();
	
	locLanguageData = (char**)malloc( sizeof(char*) * L_NUM );
	
	switch ( locId )
	{
		case 0:
			locAddStr(&i, L0_MENU_HI_STR1);
		  locAddStr(&i, L0_MENU_HI_STR2);
		  locAddStr(&i, L0_MENU_OK_STR);
		  locAddStr(&i, L0_MENU_MENU1_STR1);
		  locAddStr(&i, L0_MENU_MENU1_STR2);
		  locAddStr(&i, L0_MENU_MENU2_STR1);
		  locAddStr(&i, L0_MENU_MENU2_STR2);
		  locAddStr(&i, L0_MENU_MENU3_STR1);
		  locAddStr(&i, L0_MENU_MENU3_STR2);
		  locAddStr(&i, L0_MENU_MENU4_STR1);
		  locAddStr(&i, L0_MENU_MENU4_STR2);
		  locAddStr(&i, L0_MENU_MENU2_SUB1_STR1);
		  locAddStr(&i, L0_MENU_MENU2_SUB1_STR2);
		  locAddStr(&i, L0_MENU_MENU2_SUB2_STR1);
		  locAddStr(&i, L0_MENU_MENU2_SUB3_STR1);
		  locAddStr(&i, L0_MENU_MENU2_SUB4_STR1);
		  locAddStr(&i, L0_MENU_MENU2_SUB5_STR1);
		  locAddStr(&i, L0_MENU_MENU2_SUB5_STR2);
		  locAddStr(&i, L0_MENU_MENU2_SUB6_STR1);
		  locAddStr(&i, L0_MENU_MENU2_SUB7_STR1);
		  locAddStr(&i, L0_MENU_MENU3_SUB1_STR1);
		  locAddStr(&i, L0_MENU_MENU3_SUB1_STR2);
		  locAddStr(&i, L0_MENU_MENU3_SUB2_STR1);
      locAddStr(&i, L0_MENU_MENU3_SUB3_STR1);
			locAddStr(&i, L0_MENU_MENU3_SUB3_STR2);
			locAddStr(&i, L0_MENU_MENU3_SUB3_STR3);
	    locAddStr(&i, L0_MENU_MENU3_SUB3_STR4);			
		  locAddStr(&i, L0_MENU_MENU3_SUB4_STR1);
		  locAddStr(&i, L0_MENU_MENU3_SUB4_STR2);
		  locAddStr(&i, L0_MENU_MENU3_SUB4_STR3);
      locAddStr(&i, L0_MENU_MENU3_SUB5_STR1);
			locAddStr(&i, L0_MENU_MENU3_SUB5_STR2);
			locAddStr(&i, L0_MENU_MENU3_SUB5_STR3);
	    locAddStr(&i, L0_MENU_MENU3_SUB5_STR4);
	    locAddStr(&i, L0_MENU_MENU3_SUB6_STR1);
      locAddStr(&i, L0_MENU_MENU3_SUB6_STR2);
			locAddStr(&i, L0_MENU_MENU3_SUB6_STR3);			
			locAddStr(&i, L0_MENU_MENU4_SUB1_STR1);
			locAddStr(&i, L0_MENU_MENU4_SUB1_STR2);
			locAddStr(&i, L0_MENU_MENU4_SUB1_STR3);
			locAddStr(&i, L0_MENU_MENU4_SUB1_STR4);
		break;
		
		case 1:
			locAddStr(&i, L1_MENU_HI_STR1);
		  locAddStr(&i, L1_MENU_HI_STR2);
		  locAddStr(&i, L1_MENU_OK_STR);
		  locAddStr(&i, L1_MENU_MENU1_STR1);
		  locAddStr(&i, L1_MENU_MENU1_STR2);
		  locAddStr(&i, L1_MENU_MENU2_STR1);
		  locAddStr(&i, L1_MENU_MENU2_STR2);
		  locAddStr(&i, L1_MENU_MENU3_STR1);
		  locAddStr(&i, L1_MENU_MENU3_STR2);
		  locAddStr(&i, L1_MENU_MENU4_STR1);
		  locAddStr(&i, L1_MENU_MENU4_STR2);
		  locAddStr(&i, L1_MENU_MENU2_SUB1_STR1);
		  locAddStr(&i, L1_MENU_MENU2_SUB1_STR2);
		  locAddStr(&i, L1_MENU_MENU2_SUB2_STR1);
		  locAddStr(&i, L1_MENU_MENU2_SUB3_STR1);
		  locAddStr(&i, L1_MENU_MENU2_SUB4_STR1);
		  locAddStr(&i, L1_MENU_MENU2_SUB5_STR1);
		  locAddStr(&i, L1_MENU_MENU2_SUB5_STR2);
		  locAddStr(&i, L1_MENU_MENU2_SUB6_STR1);
		  locAddStr(&i, L1_MENU_MENU2_SUB7_STR1);
		  locAddStr(&i, L1_MENU_MENU3_SUB1_STR1);
		  locAddStr(&i, L1_MENU_MENU3_SUB1_STR2);
		  locAddStr(&i, L1_MENU_MENU3_SUB2_STR1);
      locAddStr(&i, L1_MENU_MENU3_SUB3_STR1);
			locAddStr(&i, L1_MENU_MENU3_SUB3_STR2);
			locAddStr(&i, L1_MENU_MENU3_SUB3_STR3);
	    locAddStr(&i, L1_MENU_MENU3_SUB3_STR4);
		  locAddStr(&i, L1_MENU_MENU3_SUB4_STR1);
		  locAddStr(&i, L1_MENU_MENU3_SUB4_STR2);
		  locAddStr(&i, L1_MENU_MENU3_SUB4_STR3);
      locAddStr(&i, L1_MENU_MENU3_SUB5_STR1);
			locAddStr(&i, L1_MENU_MENU3_SUB5_STR2);
			locAddStr(&i, L1_MENU_MENU3_SUB5_STR3);
	    locAddStr(&i, L1_MENU_MENU3_SUB5_STR4);
	    locAddStr(&i, L1_MENU_MENU3_SUB6_STR1);
      locAddStr(&i, L1_MENU_MENU3_SUB6_STR2);
			locAddStr(&i, L1_MENU_MENU3_SUB6_STR3);
			locAddStr(&i, L1_MENU_MENU4_SUB1_STR1);
			locAddStr(&i, L1_MENU_MENU4_SUB1_STR2);
			locAddStr(&i, L1_MENU_MENU4_SUB1_STR3);
			locAddStr(&i, L1_MENU_MENU4_SUB1_STR4);
		break;
	};
	
	return 1;
}
