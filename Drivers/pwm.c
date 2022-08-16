#include "pwm.h"
#include "time.h"

//#define PWM_FREQUENCY 187500
//#define PWM_TOP (F_CPU / PWM_FREQUENCY)
//#define PWM_DUTY_MAX PWM_TOP
//#define PWM_DUTY_MIN 0
//#define PWM_DUTY_RANGE (PWM_DUTY_MAX - PWM_DUTY_MIN)

#define MAX_2(a, b) (((a) > (b)) ? (a) : (b))
#define MIN_2(a, b) (((b) > (a)) ? (a) : (b))

bool pwm_init_tcc(Tcc* tcc, uint32_t prescaler) {
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
	tcc->CTRLA.reg = TCC_CTRLA_PRESCALER(prescaler);
	// set mode to normal pwm
	tcc->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
	while (tcc->SYNCBUSY.bit.WAVE);
	
	return true;
}

bool pwm_init_tc(Tc* tc, uint32_t prescaler) {
	switch ((uint32_t)tc) {
		case (uint32_t)TC3:
		PM->APBCMASK.bit.TC3_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3;
		break;
		case (uint32_t)TC4:
		PM->APBCMASK.bit.TC4_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC4_TC5;
		break;
		case (uint32_t)TC5:
		PM->APBCMASK.bit.TC5_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC4_TC5;
		break;
		#if defined(TC6) && defined(TC7)
		case (uint32_t)TC6:
		PM->APBCMASK.bit.TC6_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC6_TC7;
		case (uint32_t)TC7:
		PM->APBCMASK.bit.TC7_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC6_TC7;
		#endif
		default: // error, invalid tcc
		return false;
		break;
	}
	
	// wait for sync
	while (GCLK->STATUS.bit.SYNCBUSY);
	
	// set prescaler to no divide
	tc->COUNT8.CTRLA.reg = TC_CTRLA_PRESCALER(prescaler) | TC_CTRLA_WAVEGEN_NPWM | TC_CTRLA_MODE_COUNT8;
	
	return true;
}

bool pwm_enable_tcc(Tcc* tcc) {
	// check tcc
	switch ((uint32_t)tcc) {
		case (uint32_t)TCC0:
		case (uint32_t)TCC1:
		case (uint32_t)TCC2:
		break;
		default: // error, invalid tcc
		return false;
		break;
	}
	tcc->CTRLA.bit.ENABLE = 1;
	while (tcc->SYNCBUSY.bit.ENABLE);
	return true;
}

bool pwm_enable_tc(Tc* tc) {
	// check tcc
	switch ((uint32_t)tc) {
		case (uint32_t)TC3:
		case (uint32_t)TC4:
		case (uint32_t)TC5:
		#if defined(TC6) && defined(TC7)
		case (uint32_t)TC6:
		case (uint32_t)TC7:
		#endif
		break;
		default: // error, invalid tc
		return false;
		break;
	}
	tc->COUNT8.CTRLA.bit.ENABLE = 1;
	while (tc->COUNT8.STATUS.bit.SYNCBUSY);
	return true;
}

int pwm_get_div_val(uint32_t prescaler) {
	prescaler &= 0x7;
	switch (prescaler) {
		case PWM_PRESCALER_DIV1: return 1;
		case PWM_PRESCALER_DIV2: return 2;
		case PWM_PRESCALER_DIV4: return 4;
		case PWM_PRESCALER_DIV8: return 8;
		case PWM_PRESCALER_DIV16: return 16;
		case PWM_PRESCALER_DIV64: return 64;
		case PWM_PRESCALER_DIV256: return 256;
		case PWM_PRESCALER_DIV1024: return 1024;
	}
	// inaccessible
	return 0;
}

bool pwm_set_frequency_tcc(Tcc* tcc, float frequency) {
	uint32_t maxval = 0;
	switch ((uint32_t)tcc) {
		// TCC0 and TCC1 are 24 bit
		case (uint32_t)TCC0:
		case (uint32_t)TCC1:
		maxval = 0x00FFFFFF;
		break;
		// TCC2 is 16 bit
		case (uint32_t)TCC2:
		maxval = 0x0000FFFF;
		break;
		// return false if invalid tcc
		default:
		return false;
		break;
	}
	int div = pwm_get_div_val(tcc->CTRLA.bit.PRESCALER);
	uint32_t per = (uint32_t)((float)F_CPU / (frequency * div));
	// calculated value is outside range for given prescaler
	if (per > maxval) return false;
	// else set value
	tcc->PER.bit.PER = per;
	return true;
}

bool pwm_set_frequency_tc(Tc* tc, float frequency) {
	// check tcc
	switch ((uint32_t)tc) {
		case (uint32_t)TC3:
		case (uint32_t)TC4:
		case (uint32_t)TC5:
		#if defined(TC6) && defined(TC7)
		case (uint32_t)TC6:
		case (uint32_t)TC7:
		#endif
		break;
		default: // error, invalid tc
		return false;
		break;
	}
	int div = pwm_get_div_val(tc->COUNT8.CTRLA.bit.PRESCALER);
	uint32_t per = (uint32_t)((float)F_CPU / (frequency * div));
	// calculated value is outside range for given prescaler
	if (per > 0xFF) return false;
	// else set value
	tc->COUNT8.PER.bit.PER = per;
	return true;
}

//void pwm_write_tcc(Tcc* tcc, int channel, float value) {
	//if (value < 0) value = 0;
	//// calculate PWM value
	//uint32_t pwm_val = value * PWM_DUTY_RANGE + PWM_DUTY_MIN;
	//// check it is within range
	//pwm_val = (pwm_val <= PWM_DUTY_MAX) ? pwm_val : PWM_DUTY_MAX;
	//pwm_val = (pwm_val >= PWM_DUTY_MIN) ? pwm_val : PWM_DUTY_MIN;
	//// write value
	//tcc->CC[channel].bit.CC = pwm_val;
//}

bool pwm_set_duty_tcc(Tcc* tcc, int channel, float duty) {
	// check tcc
	switch ((uint32_t)tcc) {
		case (uint32_t)TCC0:
		case (uint32_t)TCC1:
		case (uint32_t)TCC2:
		break;
		default: // error, invalid tcc
		return false;
		break;
	}
	// clamp value
	duty = MAX_2(0.0f, duty);
	duty = MIN_2(1.0f, duty);
	// calculate CC value
	uint32_t pwm_val = duty * tcc->PER.bit.PER;
	// check pwm_val again in case of FP errors
	if (pwm_val > tcc->PER.bit.PER) pwm_val = tcc->PER.bit.PER;
	// assign value
	tcc->CC[channel].bit.CC = pwm_val;
	return true;
}

bool pwm_set_duty_tc(Tc* tc, int channel, float duty) {
	// check tcc
	switch ((uint32_t)tc) {
		case (uint32_t)TC3:
		case (uint32_t)TC4:
		case (uint32_t)TC5:
		#if defined(TC6) && defined(TC7)
		case (uint32_t)TC6:
		case (uint32_t)TC7:
		#endif
		break;
		default: // error, invalid tc
		return false;
		break;
	}
	// clamp value
	duty = MAX_2(0.0f, duty);
	duty = MIN_2(1.0f, duty);
	// calculate CC value
	uint32_t pwm_val = duty * tc->COUNT8.PER.bit.PER;
	// check pwm_val again in case of FP errors
	if (pwm_val > tc->COUNT8.PER.bit.PER) pwm_val = tc->COUNT8.PER.bit.PER;
	// assign value
	tc->COUNT8.CC[channel].bit.CC = pwm_val;
	return true;
}