#ifndef PWM_H
#define PWM_H

#include <stdbool.h>
#include "samd21.h"

// values are identical for TCC and TC
#define PWM_PRESCALER_DIV1 TCC_CTRLA_PRESCALER_DIV1_Val
#define PWM_PRESCALER_DIV2 TCC_CTRLA_PRESCALER_DIV2_Val
#define PWM_PRESCALER_DIV4 TCC_CTRLA_PRESCALER_DIV4_Val
#define PWM_PRESCALER_DIV8 TCC_CTRLA_PRESCALER_DIV8_Val
#define PWM_PRESCALER_DIV16 TCC_CTRLA_PRESCALER_DIV16_Val
#define PWM_PRESCALER_DIV64 TCC_CTRLA_PRESCALER_DIV64_Val
#define PWM_PRESCALER_DIV256 TCC_CTRLA_PRESCALER_DIV256_Val
#define PWM_PRESCALER_DIV1024 TCC_CTRLA_PRESCALER_DIV1024_Val

bool pwm_init_tcc(Tcc* tcc, uint32_t prescaler);
bool pwm_init_tc(Tc* tc, uint32_t prescaler);

bool pwm_enable_tcc(Tcc* tcc);
bool pwm_enable_tc(Tc* tc);

bool pwm_set_frequency_tcc(Tcc* tcc, float frequency);
bool pwm_set_frequency_tc(Tc* tc, float frequency);

bool pwm_set_duty_tcc(Tcc* tcc, int channel, float duty);
bool pwm_set_duty_tc(Tc* tc, int channel, float duty);

#endif