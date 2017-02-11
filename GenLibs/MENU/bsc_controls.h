
#ifndef CONTROLS_LIB
#define CONTROLS_LIB

#include <stdint.h>

#define VALCODER_ERROR 0x0F
#define SYS_EVENTS_DATA uint32_t

enum
{
	EVENT_BUTTON1_CLICK = 1,
	EVENT_BUTTON2_CLICK = 2,
	EVENT_BUTTON3_CLICK = 4,
	EVENT_BUTTON4_CLICK = 8,
	EVENT_BUTTON5_CLICK = 16,
	EVENT_BUTTON6_CLICK = 32,
	EVENT_VALCODER_BUTTON_CLICK = 64,
	EVENT_VALCODER_BUTTON_PRESSED = 128,
	EVENT_VALCODER_CCW = 256,
	EVENT_VALCODER_CW  = 512		
};

typedef enum
{
	UPDEVENTS_GETEVENTS  = 1,
	UPDEVENTS_HANDLERS_LAUNCH = 2
} UpdateEventsParams;

uint8_t ControlsDataClear(void);
uint8_t ControlsRegNewButton(char _portLetter, uint8_t _pinNumber, void (*_click_func)(void),
                             SYS_EVENTS_DATA _event_click_code, SYS_EVENTS_DATA _event_pressed_code);
uint8_t ControlsRegNewValcoder(char _portLetterCCW, uint8_t _pinNumberCCW,
	                             char _portLetterCW, uint8_t _pinNumberCW,
															 void (*_ccw_func)(void), SYS_EVENTS_DATA _event_ccw_code,
															 void (*_cw_func)(void), SYS_EVENTS_DATA _event_cw_code);																 
SYS_EVENTS_DATA ControlsUpdateEvents(uint32_t _params);

#endif
