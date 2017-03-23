
#ifndef GEN_TIMERS
#define GEN_TIMERS

#include <stdint.h>

uint8_t GenInitTimer1ForSignalEmul(void);
uint8_t GenInitTimer1ForPWM(void);
uint8_t GenInitSignalOnTimer1(uint8_t _signal_type);
uint8_t GenUpdateSignal(const uint16_t _tim_prescaler, const uint16_t _tim_arr, const uint16_t _tim_steps_ccr,
	                      const double _power_k, const double _center_k, const uint32_t _transistorsMinTimeNS,
												const uint32_t _transistorsDeadTimeNS, const uint8_t _signal_type);
uint8_t GenCheckSignalConfig(const int32_t _tim_prescaler, const int32_t _tim_arr, const int32_t _tim_steps_ccr,
	                           const double _power_k, const double _center_k, const uint32_t _transistorsMinTimeNS,
												     const uint32_t _transistorsDeadTimeNS, const uint8_t _signal_type);
uint16_t GenGetPrescalerValue(const double _freq_pwm, const uint8_t _signal_type);
uint16_t GenGetARRValueFromFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const uint8_t _signal_type);
uint16_t GenGetStepsCCRValueFromFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal,
	                                   const uint16_t _tim_arr, const double _power_k, const uint32_t _transistorsMinTimeNS,
																		 const uint32_t _transistorsDeadTimeNS, const uint8_t _signal_type);
double GenGetPWMFreqValueFromTimer(const uint16_t _tim_prescaler, const uint16_t _tim_arr, const uint8_t _signal_type);
double GenGetSignalFreqValueFromTimer(const uint16_t _tim_prescaler, const uint16_t _tim_arr, const uint16_t _tim_steps_ccr, const uint8_t _signal_type);																		 
uint8_t GenCheckFrequencyAvailable(const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type);
																		 
double GetRealAvailablePWMFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type);
double GetNextAvailablePWMFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type);
double GetPrevAvailablePWMFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type);
double GetRealAvailableSignalFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type);
double GetNextAvailableSignalFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type);
double GetPrevAvailableSignalFreq(const uint16_t _tim_prescaler, const double _freq_pwm, const double _freq_signal, const uint8_t _signal_type);

#endif
