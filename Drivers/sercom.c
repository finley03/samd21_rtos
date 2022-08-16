#include "sercom.h"

//// check if it is an E series samd21 (32 pins)
//#if defined(__SAMD21E15A__) || defined(__ATSAMD21E15A__) ||\
	//defined(__SAMD21E16A__) || defined(__ATSAMD21E16A__) ||\
	//defined(__SAMD21E17A__) || defined(__ATSAMD21E17A__) ||\
	//defined(__SAMD21E18A__) || defined(__ATSAMD21E18A__)
//#define SAMD21E
//#endif

bool sercom_check(Sercom* sercom) {
	switch ((uint32_t)sercom) {
		case (uint32_t)SERCOM0:
		case (uint32_t)SERCOM1:
		case (uint32_t)SERCOM2:
		case (uint32_t)SERCOM3:
		#if defined(SERCOM4) && defined(SERCOM5)
		case (uint32_t)SERCOM4:
		case (uint32_t)SERCOM5:
		#endif
		return true;
		break;
		default:
		return false;
		break;
	}
}

bool sercom_init(Sercom* sercom) {
	// send power and clock to SERCOM
	switch ((uint32_t)sercom) {
		case (uint32_t)SERCOM0:
		PM->APBCMASK.bit.SERCOM0_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_SERCOM0_CORE;
		break;
		
		case (uint32_t)SERCOM1:
		PM->APBCMASK.bit.SERCOM1_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_SERCOM1_CORE;
		break;
		
		case (uint32_t)SERCOM2:
		PM->APBCMASK.bit.SERCOM2_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_SERCOM2_CORE;
		break;
		
		case (uint32_t)SERCOM3:
		PM->APBCMASK.bit.SERCOM3_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_SERCOM3_CORE;
		break;
		
		#if defined(SERCOM4) && defined(SERCOM5)
		case (uint32_t)SERCOM4:
		PM->APBCMASK.bit.SERCOM4_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_SERCOM4_CORE;
		break;
		
		case (uint32_t)SERCOM5:
		PM->APBCMASK.bit.SERCOM5_ = 1;
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_SERCOM5_CORE;
		break;
		#endif
		
		default:
		return false;
		break;
	}
	
	while(GCLK->STATUS.bit.SYNCBUSY);
	
	return true;
}