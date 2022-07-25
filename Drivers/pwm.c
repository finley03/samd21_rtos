#include "pwm.h"
#include "time.h"

#define PWM_FREQUENCY 187500
#define PWM_TOP (F_CPU / PWM_FREQUENCY)
#define PWM_DUTY_MAX PWM_TOP
#define PWM_DUTY_MIN 0
#define PWM_DUTY_RANGE (PWM_DUTY_MAX - PWM_DUTY_MIN)

bool pwm_init_out(Tcc* tcc) {
	switch ((uint32_t)tcc) {
		case (uint32_t)TCC0:
		PM->APBCMASK.bit.TCC0_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC0_TCC1;
		break;
		case (uint32_t)TCC1:
		PM->APBCMASK.bit.TCC1_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC0_TCC1;
		break;
		case (uint32_t)TCC2:
		PM->APBCMASK.bit.TCC2_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3;
		break;
		default: // error, invalid tcc
		return false;
		break;
	}
	
	// wait for sync
	while (GCLK->STATUS.bit.SYNCBUSY);
	
	// set prescaler to no divide
	tcc->CTRLA.reg = TCC_CTRLA_PRESCALER_DIV1;
	// set mode to normal pwm
	tcc->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
	while (tcc->SYNCBUSY.bit.WAVE);
	// set top value of PWM
	tcc->PER.bit.PER = PWM_TOP;
	while (tcc->SYNCBUSY.bit.PER);
	
	// init cc counters
	tcc->CC[0].bit.CC = PWM_DUTY_MIN;
	while (tcc->SYNCBUSY.bit.CC0);
	tcc->CC[1].bit.CC = PWM_DUTY_MIN;
	while (tcc->SYNCBUSY.bit.CC1);
	if (tcc == TCC0) {
		tcc->CC[2].bit.CC = PWM_DUTY_MIN;
		while (tcc->SYNCBUSY.bit.CC2);
		tcc->CC[3].bit.CC = PWM_DUTY_MIN;
		while (tcc->SYNCBUSY.bit.CC3);
	}
	
	// enable PWM
	tcc->CTRLA.bit.ENABLE = 1;
	while (tcc->SYNCBUSY.bit.ENABLE);
	
	return true;
}

void pwm_write(Tcc* tcc, int channel, float value) {
	if (value < 0) value = 0;
	// calculate PWM value
	uint32_t pwm_val = value * PWM_DUTY_RANGE + PWM_DUTY_MIN;
	// check it is within range
	pwm_val = (pwm_val <= PWM_DUTY_MAX) ? pwm_val : PWM_DUTY_MAX;
	pwm_val = (pwm_val >= PWM_DUTY_MIN) ? pwm_val : PWM_DUTY_MIN;
	// write value
	tcc->CC[channel].bit.CC = pwm_val;
}