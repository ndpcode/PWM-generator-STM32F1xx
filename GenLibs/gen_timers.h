
#ifndef GEN_TIMERS
#define GEN_TIMERS

#include <stdint.h>

uint8_t GenInitTimers(void);
uint8_t updateSignal(uint32_t _freq_pwm, uint32_t _freq_signal, double _power_k, uint8_t _signal_type);

#endif
