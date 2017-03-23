
#ifndef GEN_LOCALIZATION
#define GEN_LOCALIZATION

#include "settings.h"
#include <stdint.h>

#define en_US 1
#define ru_RU 2
#define locLangMin en_US
#define locLangMax ru_RU

#define L_NUM 41

#define L0_MENU_HI_STR1 "Hi!"
#define L0_MENU_HI_STR2 "******"
#define L0_MENU_OK_STR "OK"
#define L0_MENU_MENU1_STR1 "PWM frequency "
#define L0_MENU_MENU1_STR2 "Signal frequency "
#define L0_MENU_MENU2_STR1 "Main config \x7E"
#define L0_MENU_MENU2_STR2 " Press \xD9 or \xDA button to enter the MAIN config "
#define L0_MENU_MENU3_STR1 "Extra config \x7E"
#define L0_MENU_MENU3_STR2 " Press \xD9 or \xDA button to enter the EXTRA config "
#define L0_MENU_MENU4_STR1 "Save config?"
#define L0_MENU_MENU4_STR2 " Press \xD9 or \xDA button for save "
#define L0_MENU_MENU2_SUB1_STR1 "PWM f"
#define L0_MENU_MENU2_SUB1_STR2 "step"
#define L0_MENU_MENU2_SUB2_STR1 "Sign f"
#define L0_MENU_MENU2_SUB3_STR1 "Power factor"
#define L0_MENU_MENU2_SUB4_STR1 "Center offset"
#define L0_MENU_MENU2_SUB5_STR1 "Prescaler"
#define L0_MENU_MENU2_SUB5_STR2 "TIM f"
#define L0_MENU_MENU2_SUB6_STR1 "ARReg"
#define L0_MENU_MENU2_SUB7_STR1 "CRReg"
#define L0_MENU_MENU3_SUB1_STR1 "Dead time"
#define L0_MENU_MENU3_SUB1_STR2 "ns"
#define L0_MENU_MENU3_SUB2_STR1 "Min pulse time"
#define L0_MENU_MENU3_SUB3_STR1 "Signal  type"
#define L0_MENU_MENU3_SUB3_STR2 "Sine"
#define L0_MENU_MENU3_SUB3_STR3 "Triangle"
#define L0_MENU_MENU3_SUB3_STR4 "Square"
#define L0_MENU_MENU3_SUB4_STR1 "Signal  update"
#define L0_MENU_MENU3_SUB4_STR2 "Auto"
#define L0_MENU_MENU3_SUB4_STR3 "Manually"
#define L0_MENU_MENU3_SUB5_STR1 "Show frequency"
#define L0_MENU_MENU3_SUB5_STR2 "Simplistic"
#define L0_MENU_MENU3_SUB5_STR3 "Only real"
#define L0_MENU_MENU3_SUB5_STR4 "Timer ctrl"
#define L0_MENU_MENU3_SUB6_STR1 "Language"
#define L0_MENU_MENU3_SUB6_STR2 "English"
#define L0_MENU_MENU3_SUB6_STR3 "Russian"
#define L0_MENU_MENU4_SUB1_STR1 "Config"
#define L0_MENU_MENU4_SUB1_STR2 "......"
#define L0_MENU_MENU4_SUB1_STR3 "saved!"
#define L0_MENU_MENU4_SUB1_STR4 "failed!" //41


#define L1_MENU_HI_STR1 "\xA8\x70\xB8\xB3\x65\xBF\x21" //"������!"
#define L1_MENU_HI_STR2 "******"
#define L1_MENU_OK_STR "\xA1\x6F\xBF\x6F\xB3\x6F" //"������"
#define L1_MENU_MENU1_STR1 "\xAB\x61\x63\xBF\x6F\xBF\x61\x20\xAC\xA5\x4D\x20" //"������� ��� "
#define L1_MENU_MENU1_STR2 "\xAB\x61\x63\xBF\x6F\xBF\x61\x20\x63\xB8\xB4\xBD\x61\xBB\x61\x20" //"������� ������� "
#define L1_MENU_MENU2_STR1 "\x4F\x63\xBD\x6F\xB3\xBD\x6F\x65\x20\x7E" //"�������� 0x7E"
//" ������� * ��� * ��� ����� � �������� ��������� "
#define L1_MENU_MENU2_STR2 "\x20\x48\x61\xB6\xBC\xB8\xBF\x65\x20\xD9\x20\xB8\xBB\xB8\x20\xDA\x20\xE3\xBB\xC7\x20\xB3\x78\x6F\xE3\x61\x20\xB3\x20\x6F\x63\xBD\x6F\xB3\xBD\xC3\x65\x20\xBD\x61\x63\xBF\x70\x6F\xB9\xBA\xB8\x20"
#define L1_MENU_MENU3_STR1 "\xE0\x6F\xBE\x6F\xBB\xBD\xB8\xBF\x65\xBB\xC4\xBD\x6F\x20\x7E" //"������������� 0x7E"
//" ������� * ��� * ��� ����� � �������������� ��������� "
#define L1_MENU_MENU3_STR2 "\x20\x48\x61\xB6\xBC\xB8\xBF\x65\x20\xD9\x20\xB8\xBB\xB8\x20\xDA\x20\xE3\xBB\xC7\x20\xB3\x78\x6F\xE3\x61\x20\xB3\x20\xE3\x6F\xBE\x6F\xBB\xBD\xB8\xBF\x65\xBB\xC4\xBD\xC3\x65\x20\xBD\x61\x63\xBF\x70\x6F\xB9\xBA\xB8\x20"
#define L1_MENU_MENU4_STR1 "\x20\x43\x6F\x78\x70\x61\xBD\xB8\xBF\xC4\x20\xBD\x61\x63\xBF\x70\x6F\xB9\xBA\xB8\x3F\x20" //" ��������� ���������? "
//" ������� * ��� * ��� ���������� "
#define L1_MENU_MENU4_STR2 "\x20\x48\x61\xB6\xBC\xB8\xBF\x65\x20\xD9\x20\xB8\xBB\xB8\x20\xDA\x20\xE3\xBB\xC7\x20\x63\x6F\x78\x70\x61\xBD\x65\xBD\xB8\xC7\x20"
#define L1_MENU_MENU2_SUB1_STR1 "\xAC\xA5\x4D\x20\x66" //"��� f"
#define L1_MENU_MENU2_SUB1_STR2 "\xC1\x61\xB4" //"���"
#define L1_MENU_MENU2_SUB2_STR1 "\x43\xB8\xB4\xBD\x20\x66" //"���� f"
#define L1_MENU_MENU2_SUB3_STR1 "\x20\x4B\x6F\xC5\xE4\xE4\xB8\xE5\xB8\x65\xBD\xBF\x20\xBC\x6F\xE6\xBD\x6F\x63\xBF\xB8\x20" //" ����������� �������� "
#define L1_MENU_MENU2_SUB4_STR1 "\x43\xBC\x65\xE6\x65\xBD\xB8\x65\x20\xE5\x65\xBD\xBF\x70\x61" //"�������� ������"
#define L1_MENU_MENU2_SUB5_STR1 "\xE0\x65\xBB\xB8\xBF\x65\xBB\xC4" //"��������"
#define L1_MENU_MENU2_SUB5_STR2 "\x54\x41\xA6\x4D\x20\x66" //"���� f"
#define L1_MENU_MENU2_SUB6_STR1 "ARReg"
#define L1_MENU_MENU2_SUB7_STR1 "CRReg"
#define L1_MENU_MENU3_SUB1_STR1 "Dead time"
#define L1_MENU_MENU3_SUB1_STR2 "\xBD\x63" //"��"
#define L1_MENU_MENU3_SUB2_STR1 "\x4D\xB8\xBD\x20\x74\x20\xB8\xBC\xBE\x79\xBB\xC4\x63\x61" //"��� t ��������"
#define L1_MENU_MENU3_SUB3_STR1 "\x54\xB8\xBE\x20\x63\xB8\xB4\xBD\x61\xBB\x61" //"��� �������"
#define L1_MENU_MENU3_SUB3_STR2 "\x43\xB8\xBD\x79\x63" //"�����"
#define L1_MENU_MENU3_SUB3_STR3 "\x54\x70\x65\x79\xB4\x6F\xBB\xC4\xBD\xC3\xB9" //"�����������"
#define L1_MENU_MENU3_SUB3_STR4 "\x4D\x65\x61\xBD\xE3\x70" //"������"
#define L1_MENU_MENU3_SUB4_STR1 "\x4F\xB2\xBD\x6F\xB3\xBB\x65\xBD\xB8\x65\x20\x63\xB8\xB4\xBD\x61\xBB\x61" //"���������� �������"
#define L1_MENU_MENU3_SUB4_STR2 "\x41\xB3\xBF\x6F" //"����"
#define L1_MENU_MENU3_SUB4_STR3 "\x42\x70\x79\xC0\xBD\x79\xC6" //"�������"
#define L1_MENU_MENU3_SUB5_STR1 "\xA8\x6F\xBA\x61\xB7\xC3\xB3\x61\xBF\xC4\x20\xB7\xBD\x61\xC0\x65\xBD\xB8\xC7\x20\xC0\x61\x63\xBF\x6F\xBF\xC3" //"���������� �������� �������"
#define L1_MENU_MENU3_SUB5_STR2 "\x4F\xB2\xC3\xC0\xBD\xC3\x65" //"�������"
#define L1_MENU_MENU3_SUB5_STR3 "\x50\x65\x61\xBB\xC4\xBD\xC3\x65" //"��������"
#define L1_MENU_MENU3_SUB5_STR4 "\xA9\xBE\x70\x20\xBF\x61\xB9\xBC\x65\x70" //"��� ������"
#define L1_MENU_MENU3_SUB6_STR1 "\xB1\xB7\xC3\xBA" //"����"
#define L1_MENU_MENU3_SUB6_STR2 "\x41\xBD\xB4\xBB\xB8\x63\xB9\xBA\xB8\xB9" //"����������"
#define L1_MENU_MENU3_SUB6_STR3 "\x50\x79\x63\x63\xBA\xB8\xB9" //"�������"
#define L1_MENU_MENU4_SUB1_STR1 "\x48\x61\x63\xBF\x70\x6F\xB9\xBA\xB8" //"���������"
#define L1_MENU_MENU4_SUB1_STR2 "......"
#define L1_MENU_MENU4_SUB1_STR3 "\x63\x6F\x78\x70\x61\xBD\x65\xBD\xC3\x21" //"���������!"
#define L1_MENU_MENU4_SUB1_STR4 "\x6F\xC1\xB8\xB2\xBA\x61\x21" //"������!"

uint8_t GenChangeLocalization(uint8_t locId);

#endif
