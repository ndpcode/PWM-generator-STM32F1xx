
#ifndef GEN_MENU
#define GEN_MENU

#include <stdint.h>
#include "bsc_controls.h"

void MenuTransitionDraw(const uint16_t frameNum);
//Menu HI
uint8_t MenuHiDraw(const uint8_t frameNum);
//Menu OK
uint8_t MenuOKDraw(const uint8_t frameNum);

//Menu1 - Start Menu
uint8_t Menu1_StartMenuDraw(const uint8_t frameNum);
uint8_t Menu1_StartMenuEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);

//Menu2 - Main Menu
uint8_t Menu2_MainMenuDraw(const uint8_t frameNum);
uint8_t Menu2_MainMenuEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu3 - Extra Menu
uint8_t Menu3_ExtraMenuDraw(const uint8_t frameNum);
uint8_t Menu3_ExtraMenuEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu4 - Save Menu
uint8_t Menu4_SaveMenuDraw(const uint8_t frameNum);
uint8_t Menu4_SaveMenuEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);

//Menu2 - SubMenu1 - Change PWM Freq
uint8_t Menu2_SubMenu1_ChangePWMFreqDraw(const uint8_t frameNum);
uint8_t Menu2_SubMenu1_ChangePWMFreqEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu2 - SubMenu2 - Change Signal Freq
uint8_t Menu2_SubMenu2_ChangeSignalFreqDraw(const uint8_t frameNum);
uint8_t Menu2_SubMenu2_ChangeSignalFreqEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu2 - SubMenu3 - Change Signal Power
uint8_t Menu2_SubMenu3_ChangeSignalPowerDraw(const uint8_t frameNum);
uint8_t Menu2_SubMenu3_ChangeSignalPowerEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu2 - SubMenu4 - Change Signal Center
uint8_t Menu2_SubMenu4_ChangeSignalCenterDraw(const uint8_t frameNum);
uint8_t Menu2_SubMenu4_ChangeSignalCenterEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);

//Menu3 - SubMenu1 - Change Dead Time
uint8_t Menu3_SubMenu1_ChangeDeadTimeDraw(const uint8_t frameNum);
uint8_t Menu3_SubMenu1_ChangeDeadTimeEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu3 - SubMenu2 - Change Minimum Pulse Time
uint8_t Menu3_SubMenu2_ChangeMinPulseTimeDraw(const uint8_t frameNum);
uint8_t Menu3_SubMenu2_ChangeMinPulseTimeEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu3 - SubMenu3 - Change Signal Type
uint8_t Menu3_SubMenu3_ChangeSignalTypeDraw(const uint8_t frameNum);
uint8_t Menu3_SubMenu3_ChangeSignalTypeEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu3 - SubMenu4 - Change Update Type
uint8_t Menu3_SubMenu4_ChangeUpdateTypeDraw(const uint8_t frameNum);
uint8_t Menu3_SubMenu4_ChangeUpdateTypeEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu3 - SubMenu5 - Change Show Real Freq Type
uint8_t Menu3_SubMenu5_ChangeShowFreqTypeDraw(const uint8_t frameNum);
uint8_t Menu3_SubMenu5_ChangeShowFreqTypeEvents(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);

//Menu4 - SubMenu1 - Save Dialog
uint8_t Menu4_SubMenu1_SaveDialogDraw(const uint8_t frameNum);

#endif
