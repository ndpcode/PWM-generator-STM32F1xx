
#ifndef GEN_LOCALIZATION
#define GEN_LOCALIZATION

#include "settings.h"
#include <stdint.h>

#define en_US 0
#define ru_RU 1

#define L_NUM 5

#define L0_MENU_HI_STR1 "Hi!"
#define L0_MENU_HI_STR2 "******"
#define L0_MENU_OK_STR "OK"
#define L0_MENU_MENU1_STR1 "PWM frequency "
#define L0_MENU_MENU1_STR2 "Signal frequency "


#define L1_MENU_HI_STR1 "\xA8\x70\xB8\xB3\x65\xBF\x21" //"Привет!"
#define L1_MENU_HI_STR2 "******"
#define L1_MENU_OK_STR "\xA1\x6F\xBF\x6F\xB3\x6F" //"Готово"
#define L1_MENU_MENU1_STR1 "\xAB\x61\x63\xBF\x6F\xBF\x61\x20\xAC\xA5\x4D\x20" //"Частота ШИМ "
#define L1_MENU_MENU1_STR2 "\xAB\x61\x63\xBF\x6F\xBF\x61\x20\x63\xB8\xB4\xBD\x61\xBB\x61\x20" //"Частота сигнала "

uint8_t GenChangeLocalization(uint8_t locId);

#endif