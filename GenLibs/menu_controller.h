
#ifndef MENU_CONTROLLER
#define MENU_CONTROLLER

#include <stdint.h>

uint8_t RegNewMenuItem(uint8_t menu_index, uint8_t (*_menu_draw_func)(uint8_t),
                                              uint8_t (*_menu_events_func)(uint32_t));
uint8_t ChangeMenuItem(uint8_t new_index);
uint8_t MenuUpdate(void);

#endif
