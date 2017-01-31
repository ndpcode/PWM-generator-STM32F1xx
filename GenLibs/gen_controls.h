
#include <stdint.h>

#define BUTTON_EVENTS_CLICK 1
#define BUTTON_EVENTS_DOUBLE_CLICK 2
#define BUTTON_EVENTS_HOLD 4

void ButtonsDataClear(void);
void ButtonsAddNewButton(char _portLetter, uint8_t _pinNumber, void (*_click_func)(void));
void ButtonsCheck(void);
