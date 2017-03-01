
#ifndef GEN_TIMERS
#define GEN_TIMERS

#include <stdint.h>

uint8_t GenInitTimer1ForSignalEmul(void);
uint8_t GenInitTimer1ForPWM(void);
uint8_t GenInitSignalOnTimer1(uint8_t _signal_type);
uint8_t UpdateSignal(uint32_t _freq_pwm, uint32_t _freq_signal, double _power_k, double _center_k,
                     uint32_t _transistorsMinTimeNS, uint32_t _transistorsDeadTimeNS, uint8_t _signal_type);
uint8_t FrequencyCheck(uint32_t _freq_pwm, uint32_t _freq_signal);
uint32_t GetNextAvailableSignalFreq(uint32_t _freq_pwm, uint32_t _freq_signal);
uint32_t GetPrevAvailableSignalFreq(uint32_t _freq_pwm, uint32_t _freq_signal);

#endif
