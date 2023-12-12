#include "sercom.h"

bool sercom_check(sercom_registers_t* sercom) {
	switch ((uint32_t)sercom) {
		case (uint32_t)SERCOM0_REGS:
		case (uint32_t)SERCOM1_REGS:
		case (uint32_t)SERCOM2_REGS:
		case (uint32_t)SERCOM3_REGS:
		#if defined(SERCOM4_REGS) && defined(SERCOM5_REGS)
		case (uint32_t)SERCOM4_REGS:
		case (uint32_t)SERCOM5_REGS:
		#endif
		return true;
		break;
		default:
		return false;
		break;
	}
}

bool sercom_init(sercom_registers_t* sercom) {
	// send power and clock to SERCOM
	switch ((uint32_t)sercom) {
		case (uint32_t)SERCOM0_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_SERCOM0(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_ID_SERCOM0_CORE;
		break;
		
		case (uint32_t)SERCOM1_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_SERCOM1(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_ID_SERCOM1_CORE;
		break;
		
		case (uint32_t)SERCOM2_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_SERCOM2(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_ID_SERCOM2_CORE;
		break;
		
		case (uint32_t)SERCOM3_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_SERCOM3(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_ID_SERCOM3_CORE;
		break;
		
		#if defined(SERCOM4_REGS) && defined(SERCOM5_REGS)
		case (uint32_t)SERCOM4_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_SERCOM4(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_ID_SERCOM4_CORE;
		break;
		
		case (uint32_t)SERCOM5_REGS:
		PM_REGS->PM_APBCMASK |= PM_APBCMASK_SERCOM5(1);
		GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_ID_SERCOM5_CORE;
		break;
		#endif
		
		default:
		return false;
		break;
	}
	
	while(GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk);
	
	return true;
}