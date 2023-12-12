#include "pwm.h"
#include "time.h"

//#define PWM_FREQUENCY 187500
//#define PWM_TOP (F_CPU / PWM_FREQUENCY)
//#define PWM_DUTY_MAX PWM_TOP
//#define PWM_DUTY_MIN 0
//#define PWM_DUTY_RANGE (PWM_DUTY_MAX - PWM_DUTY_MIN)

#define MAX_2(a, b) (((a) > (b)) ? (a) : (b))
#define MIN_2(a, b) (((b) > (a)) ? (a) : (b))

bool pwm_init_tcc(tcc_registers_t* tcc, uint32_t prescaler) {
	switch ((uint32_t)tcc) {
		case (uint32_t)TCC0_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_TCC0(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC0_TCC1;
		break;
		case (uint32_t)TCC1_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_TCC1(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC0_TCC1;
		break;
		case (uint32_t)TCC2_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_TCC2(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3;
		break;
		default: // error, invalid tcc
		return false;
		break;
	}
	
	// wait for sync
	// while (GCLK->STATUS.bit.SYNCBUSY);
	while (GCLK_REGS->GCLK_STATUS * GCLK_STATUS_SYNCBUSY_Msk);
	
	// set prescaler to no divide
	tcc->TCC_CTRLA = TCC_CTRLA_PRESCALER(prescaler);
	// set mode to normal pwm
	tcc->TCC_WAVE = TCC_WAVE_WAVEGEN_NPWM;
	while (tcc->TCC_SYNCBUSY & TCC_SYNCBUSY_WAVE_Msk);
	
	return true;
}

bool pwm_init_tc(tc_registers_t* tc, uint32_t prescaler) {
	switch ((uint32_t)tc) {
		case (uint32_t)TC3_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_TC3(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3;
		break;
		case (uint32_t)TC4_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_TC4(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC4_TC5;
		break;
		case (uint32_t)TC5_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_TC5(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC4_TC5;
		break;
		#if defined(TC6_REGS) && defined(TC7_REGS)
		case (uint32_t)TC6_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_TC6(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC6_TC7;
		break;
		case (uint32_t)TC7_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_TC7(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC6_TC7;
		break;
		#endif
		default: // error, invalid tcc
		return false;
		break;
	}
	
	// wait for sync
	while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk);
	
	// set prescaler to no divide
	tc->COUNT8.TC_CTRLA = TC_CTRLA_PRESCALER(prescaler) | TC_CTRLA_WAVEGEN_NPWM | TC_CTRLA_MODE_COUNT8;
	
	return true;
}

bool pwm_enable_tcc(tcc_registers_t* tcc) {
	// check tcc
	switch ((uint32_t)tcc) {
		case (uint32_t)TCC0_REGS:
		case (uint32_t)TCC1_REGS:
		case (uint32_t)TCC2_REGS:
		break;
		default: // error, invalid tcc
		return false;
		break;
	}
	tcc->TCC_CTRLA |= TCC_CTRLA_ENABLE(1);
	while (tcc->TCC_SYNCBUSY & TCC_SYNCBUSY_ENABLE_Msk);
	return true;
}

bool pwm_enable_tc(tc_registers_t* tc) {
	// check tcc
	switch ((uint32_t)tc) {
		case (uint32_t)TC3_REGS:
		case (uint32_t)TC4_REGS:
		case (uint32_t)TC5_REGS:
		#if defined(TC6_REGS) && defined(TC7_REGS)
		case (uint32_t)TC6_REGS:
		case (uint32_t)TC7_REGS:
		#endif
		break;
		default: // error, invalid tc
		return false;
		break;
	}
	tc->COUNT8.TC_CTRLA |= TC_CTRLA_ENABLE(1);
	while (tc->COUNT8.TC_STATUS & TC_STATUS_SYNCBUSY_Msk);
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

bool pwm_set_frequency_tcc(tcc_registers_t* tcc, float frequency) {
	uint32_t maxval = 0;
	switch ((uint32_t)tcc) {
		// TCC0 and TCC1 are 24 bit
		case (uint32_t)TCC0_REGS:
		case (uint32_t)TCC1_REGS:
		maxval = 0x00FFFFFF;
		break;
		// TCC2 is 16 bit
		case (uint32_t)TCC2_REGS:
		maxval = 0x0000FFFF;
		break;
		// return false if invalid tcc
		default:
		return false;
		break;
	}
	int div = pwm_get_div_val((tcc->TCC_CTRLA & TCC_CTRLA_PRESCALER_Msk) >> TCC_CTRLA_PRESCALER_Pos);
	uint32_t per = (uint32_t)((float)F_CPU / (frequency * div));
	// calculated value is outside range for given prescaler
	if (per > maxval) return false;
	// else set value
	tcc->TCC_PER |= TCC_PER_PER(per);
	return true;
}

bool pwm_set_frequency_tc(tc_registers_t* tc, float frequency) {
	// check tcc
	switch ((uint32_t)tc) {
		case (uint32_t)TC3_REGS:
		case (uint32_t)TC4_REGS:
		case (uint32_t)TC5_REGS:
		#if defined(TC6_REGS) && defined(TC7_REGS)
		case (uint32_t)TC6_REGS:
		case (uint32_t)TC7_REGS:
		#endif
		break;
		default: // error, invalid tc
		return false;
		break;
	}
	int div = pwm_get_div_val((tc->COUNT8.TC_CTRLA & TC_CTRLA_PRESCALER_Msk) >> TC_CTRLA_PRESCALER_Pos);
	uint32_t per = (uint32_t)((float)F_CPU / (frequency * div));
	// calculated value is outside range for given prescaler
	if (per > 0xFF) return false;
	// else set value
	tc->COUNT8.TC_PER = TC_COUNT8_PER_PER(per);
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

bool pwm_set_duty_tcc(tcc_registers_t* tcc, int channel, float duty) {
	// check tcc
	switch ((uint32_t)tcc) {
		case (uint32_t)TCC0_REGS:
		case (uint32_t)TCC1_REGS:
		case (uint32_t)TCC2_REGS:
		break;
		default: // error, invalid tcc
		return false;
		break;
	}
	// clamp value
	duty = MAX_2(0.0f, duty);
	duty = MIN_2(1.0f, duty);
	// get per value
	uint32_t per = (tcc->TCC_PER & TCC_PER_PER_Msk) >> TCC_PER_PER_Pos;
	// calculate CC value
	uint32_t pwm_val = duty * per;
	// check pwm_val again in case of FP errors
	if (pwm_val > per) pwm_val = per;
	// assign value
	// tcc->TCC_CC[channel].bit.CC = pwm_val;
	tcc->TCC_CC[channel] |= TCC_CC_CC(pwm_val);
	return true;
}

bool pwm_set_duty_tc(tc_registers_t* tc, int channel, float duty) {
	// check tcc
	switch ((uint32_t)tc) {
		case (uint32_t)TC3_REGS:
		case (uint32_t)TC4_REGS:
		case (uint32_t)TC5_REGS:
		#if defined(TC6_REGS) && defined(TC7_REGS)
		case (uint32_t)TC6_REGS:
		case (uint32_t)TC7_REGS:
		#endif
		break;
		default: // error, invalid tc
		return false;
		break;
	}
	// clamp value
	duty = MAX_2(0.0f, duty);
	duty = MIN_2(1.0f, duty);
	// get per value
	uint8_t per = (tc->COUNT8.TC_PER & TC_COUNT8_PER_PER_Msk) >> TC_COUNT8_PER_PER_Pos;
	// calculate CC value
	uint32_t pwm_val = duty * per;
	// check pwm_val again in case of FP errors
	if (pwm_val > per) pwm_val = per;
	// assign value
	tc->COUNT8.TC_CC[channel] = TC_COUNT8_CC_CC(pwm_val);
	return true;
}