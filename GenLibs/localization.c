
#include "localization.h"
#include <stdlib.h>
#include <string.h>

char **locLanguageData;

void locLanguageDataClear(void)
{
	uint8_t i,j;
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

uint8_t GenChangeLocalization(uint8_t locId)
{
	char *_str;
	locLanguageDataClear();
	
	locLanguageData = (char**)malloc( sizeof(char*) * L_NUM );
	
	switch ( locId )
	{
		case 0:
			locLanguageData[0] = (char*)malloc( sizeof(char) * ( strlen(L0_MENU_HI_STR1) + 1) );
			strcpy(locLanguageData[0], L0_MENU_HI_STR1);
			locLanguageData[1] = (char*)malloc( sizeof(char) * ( strlen(L0_MENU_HI_STR2) + 1) );
			strcpy(locLanguageData[1], L0_MENU_HI_STR2);
			locLanguageData[2] = (char*)malloc( sizeof(char) * ( strlen(L0_MENU_OK_STR) + 1) );
			strcpy(locLanguageData[2], L0_MENU_OK_STR);
			locLanguageData[3] = (char*)malloc( sizeof(char) * ( strlen(L0_MENU_MENU1_STR1) + 1) );
			strcpy(locLanguageData[3], L0_MENU_MENU1_STR1);
			locLanguageData[4] = (char*)malloc( sizeof(char) * ( strlen(L0_MENU_MENU1_STR2) + 1) );
			strcpy(locLanguageData[4], L0_MENU_MENU1_STR2);
		break;
		
		case 1:
			locLanguageData[0] = (char*)malloc( sizeof(char) * ( strlen(L1_MENU_HI_STR1) + 1) );
			strcpy(locLanguageData[0], L1_MENU_HI_STR1);
			locLanguageData[1] = (char*)malloc( sizeof(char) * ( strlen(L1_MENU_HI_STR2) + 1) );
			strcpy(locLanguageData[1], L1_MENU_HI_STR2);
			locLanguageData[2] = (char*)malloc( sizeof(char) * ( strlen(L1_MENU_OK_STR) + 1) );
			strcpy(locLanguageData[2], L1_MENU_OK_STR);
			locLanguageData[3] = (char*)malloc( sizeof(char) * ( strlen(L1_MENU_MENU1_STR1) + 1) );
			strcpy(locLanguageData[3], L1_MENU_MENU1_STR1);
			locLanguageData[4] = (char*)malloc( sizeof(char) * ( strlen(L1_MENU_MENU1_STR2) + 1) );
			strcpy(locLanguageData[4], L1_MENU_MENU1_STR2);
		break;
	};
	
	return 1;
}