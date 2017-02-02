
#ifndef CONTROLS_LIB
#define CONTROLS_LIB

#include <stdint.h>

#define VALCODER_ERROR 0x0F

typedef enum
{
	UPDEVENTS_GETEVENTS  = 1,
	UPDEVENTS_HANDLERS_LAUNCH = 2
} UpdateEventsParams;

uint8_t ControlsDataClear(void);
uint8_t ControlsRegNewButton(char _portLetter, uint8_t _pinNumber, void (*_click_func)(void), uint32_t _event_code);
uint8_t ControlsRegNewValcoder(char _portLetterCCW, uint8_t _pinNumberCCW,
	                             char _portLetterCW, uint8_t _pinNumberCW,
															 void (*_ccw_func)(void), uint32_t _event_ccw_code,
															 void (*_cw_func)(void), uint32_t _event_cw_code);																 
uint32_t ControlsUpdateEvents(UpdateEventsParams _params);

#endif
