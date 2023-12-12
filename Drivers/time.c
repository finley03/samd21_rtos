#include "time.h"
#include <sam.h>

void set_clock_48m() {
	// set wait states for flash to 1
	// see datasheet table 37-42

	NVMCTRL_REGS->NVMCTRL_CTRLB |= NVMCTRL_CTRLB_RWS(1);

	// enable external 32khz oscillator
	SYSCTRL_REGS->SYSCTRL_XOSC32K = SYSCTRL_XOSC32K_STARTUP(0x4) | SYSCTRL_XOSC32K_EN32K(1) | SYSCTRL_XOSC32K_XTALEN(1);
	// seperate write
	SYSCTRL_REGS->SYSCTRL_XOSC32K |= SYSCTRL_XOSC32K_ENABLE(1);

	// wait until done
	while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_XOSC32KRDY_Msk));

	// configure GCLK1
	// set divide to 1 (no division)
	GCLK_REGS->GCLK_GENDIV = GCLK_GENDIV_ID(1) | GCLK_GENDIV_DIV(1);

	// set GCLK1 to use external 32k oscillator
	GCLK_REGS->GCLK_GENCTRL = GCLK_GENCTRL_ID(1) | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_IDC(1) | GCLK_GENCTRL_GENEN(1);

	// wait for data write to complete
	while(GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk);

	// send GCLK1 to DFLL
	GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_ID_DFLL48 | GCLK_CLKCTRL_GEN_GCLK1 | GCLK_CLKCTRL_CLKEN(1);

	// set up DFLL
	// mush reset value before configuration
	while(!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY_Msk));
	SYSCTRL_REGS->SYSCTRL_DFLLCTRL = SYSCTRL_DFLLCTRL_ENABLE(1);
	while(!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY_Msk));

	// set clock multiplier and trims
	// SYSCTRL_DFLLMUL_MUL = factor of multiplication
	// SYSCTRL_DFLLMUL_FSTEP = fine step
	// SYSCTRL_SFLLMUL_CSTEP = coarse step
	SYSCTRL_REGS->SYSCTRL_DFLLMUL = SYSCTRL_DFLLMUL_MUL(1465) | SYSCTRL_DFLLMUL_FSTEP(511) | SYSCTRL_DFLLMUL_CSTEP(31);

	// wait for write to finish
	while(!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY_Msk));

	// set default DFLL values from fuses
	// revisit for full understanding
	uint32_t coarse = FUSES_OTP4_WORD_1_DFLL48M_COARSE_CAL(OTP4_FUSES_REGS->FUSES_OTP4_WORD_1);
	
	SYSCTRL_REGS->SYSCTRL_DFLLVAL |= SYSCTRL_DFLLVAL_COARSE(coarse);

	while(!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY_Msk));

	// turn on DFLL and set to closed loop mode
	SYSCTRL_REGS->SYSCTRL_DFLLCTRL |= SYSCTRL_DFLLCTRL_MODE(1) | SYSCTRL_DFLLCTRL_WAITLOCK(1) | SYSCTRL_DFLLCTRL_ENABLE(1);

	// wait for frequency lock
	while(!((SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLLCKC_Msk) >> SYSCTRL_PCLKSR_DFLLLCKC_Pos) || !((SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLLCKF_Msk) >> SYSCTRL_PCLKSR_DFLLLCKF_Pos));
	// switch GCLK0 to use DFLL

	GCLK_REGS->GCLK_GENCTRL = GCLK_GENCTRL_ID(0) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_IDC(1) | GCLK_GENCTRL_GENEN(1);

	// wait for write to complete
	while(GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk);
}

//__attribute__((section(".ramfunc")))
void delay_8c(uint32_t n) {
	__asm (
	"loop:				\n" // loop for delay loop
	"	sub r0, r0, #1	\n" // n is loaded into r0, so each loop, subtract 1
	"	nop				\n"
	"	nop				\n"
	"	nop				\n"
	"	nop				\n"
	"	nop				\n"
	"	bne loop		\n" // if counter is not 0, loop
	);
}


void init_timer() {
	// never forget the bus clock
	// PM->APBCMASK.bit.TC4_ = 1;
	PM_REGS->PM_APBCMASK |= PM_APBCMASK_TC4(1);
	
	// pipe 48mhz clock to tc4/tc5 counters
	GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC4_TC5;
	while(GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk);
	
	// configure counters
	TC4_REGS->COUNT32.TC_CTRLA = TC_CTRLA_MODE_COUNT32;
	while(TC4_REGS->COUNT32.TC_STATUS & TC_STATUS_SYNCBUSY_Msk);
	
	// enable TC4
	TC4_REGS->COUNT32.TC_CTRLA |= TC_CTRLA_ENABLE(1);
	while(TC4_REGS->COUNT32.TC_STATUS & TC_STATUS_SYNCBUSY_Msk);
	
	// enable continuous read
	TC4_REGS->COUNT32.TC_READREQ = TC_READREQ_RCONT(1) | TC_READREQ_ADDR(0x10);
	while(TC4_REGS->COUNT32.TC_STATUS & TC_STATUS_SYNCBUSY_Msk);
}


void start_timer() {
	TC4_REGS->COUNT32.TC_CTRLBSET = TC_CTRLBCLR_CMD_RETRIGGER;
	while(TC4_REGS->COUNT32.TC_STATUS & TC_STATUS_SYNCBUSY_Msk);
}


uint32_t read_timer_20ns() {
	return TC4_REGS->COUNT32.TC_COUNT;
}

float read_timer_us() {
	return (float)TC4_REGS->COUNT32.TC_COUNT * TIMER_US_MULTIPLIER;
}

float read_timer_ms() {
	return (float)TC4_REGS->COUNT32.TC_COUNT * TIMER_MS_MULTIPLIER;
}

float read_timer_s() {
	return (float)TC4_REGS->COUNT32.TC_COUNT * TIMER_S_MULTIPLIER;
}


void init_timer_interrupt() {
	TC4_REGS->COUNT32.TC_INTENSET = TC_INTENSET_MC0(1);
	while(TC4_REGS->COUNT32.TC_STATUS & TC_STATUS_SYNCBUSY_Msk);
}

void timer_enable_interrupt() {
	NVIC_EnableIRQ(TC4_IRQn);
}

void timer_disable_interrupt() {
	NVIC_DisableIRQ(TC4_IRQn);
}

void timer_clear_interrupt() {
	TC4_REGS->COUNT32.TC_INTFLAG = TC_INTFLAG_MC0(1);
	NVIC_ClearPendingIRQ(TC4_IRQn);
}

void timer_set_interrupt_time(uint32_t time) {
	TC4_REGS->COUNT32.TC_CC[0] |= TC_COUNT32_CC_CC(time);
}