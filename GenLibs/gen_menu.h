
#ifndef GEN_MENU
#define GEN_MENU

#include <stdint.h>
#include "bsc_controls.h"

void MenuTransitionDraw(const uint16_t frameNum);
//Menu1 - MAIN menu
uint8_t Menu1Draw(const uint8_t frameNum);
uint8_t Menu1Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu2 - Signal freq set
uint8_t Menu2Draw(const uint8_t frameNum);
uint8_t Menu2Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu3 - PWM freq set
uint8_t Menu3Draw(const uint8_t frameNum);
uint8_t Menu3Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu4 - power K set
uint8_t Menu4Draw(const uint8_t frameNum);
uint8_t Menu4Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu5 - change type of signal
uint8_t Menu5Draw(const uint8_t frameNum);
uint8_t Menu5Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu6 - change type of update freq (manual or auto)
uint8_t Menu6Draw(const uint8_t frameNum);
uint8_t Menu6Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Menu7 - save config
uint8_t Menu7Draw(const uint8_t frameNum);
uint8_t Menu7Events(const uint16_t frameNum, SYS_EVENTS_DATA genEvents);
//Manu save config
uint8_t MenuSaveDraw(const uint8_t frameNum); 

#endif
