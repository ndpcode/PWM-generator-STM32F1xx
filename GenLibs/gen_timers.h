
#include <stdint.h>

void gen_init_timers(void);
void updateSignal(uint32_t _freq_pwm, uint32_t _freq_signal, double _power_k, uint8_t _signal_type);
void startPWMSin(uint32_t _freq_pwm, uint32_t _freq_sin);
