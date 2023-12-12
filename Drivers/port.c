#include "port.h"
#include <sam.h>

void port_set_output(uint8_t port, uint32_t pinmask) {
	PORT_REGS->GROUP[port].PORT_DIRSET = pinmask;
}

void port_set_input(uint8_t port, uint32_t pinmask) {
	PORT_REGS->GROUP[port].PORT_DIRCLR = pinmask;
}

void port_enable_input(uint8_t port, uint32_t pinmask) {
	for (int i = 0; i < 32; ++i) {
		if ((1 << i) & pinmask) PORT_REGS->GROUP[port].PORT_PINCFG[i] |= PORT_PINCFG_INEN(1);
	}
}

void port_disable_input(uint8_t port, uint32_t pinmask) {
	for (int i = 0; i < 32; ++i) {
		if ((1 << i) & pinmask) PORT_REGS->GROUP[port].PORT_PINCFG[i] &= ~PORT_PINCFG_INEN(1);
	}
}


void port_set(uint8_t port, uint32_t pinmask) {
	PORT_REGS->GROUP[port].PORT_OUTSET = pinmask;
}

void port_clear(uint8_t port, uint32_t pinmask) {
	PORT_REGS->GROUP[port].PORT_OUTCLR = pinmask;
}

void port_toggle(uint8_t port, uint32_t pinmask) {
	PORT_REGS->GROUP[port].PORT_OUTTGL = pinmask;
}


void port_wrconfig(uint8_t port, uint32_t pmux, uint32_t pinmask) {
	// PORT_WRCONFIG_DRVSTR wrconfig = {
	// 	// enable writing to pincfg
	// 	.bit.WRPINCFG = 1,
	// 	// enable writing to pmux
	// 	.bit.WRPMUX = 1,
	// 	// select pmux function
	// 	.bit.PMUX = pmux,
	// 	// enable peripheral multiplexing
	// 	.bit.PMUXEN = 1
	// };

	uint32_t wrconfig = {
		// enable writing to pincfg
		PORT_WRCONFIG_WRPINCFG(1) |
		// enable writing to pmux
		PORT_WRCONFIG_WRPMUX(1) |
		// select pmux function
		PORT_WRCONFIG_PMUX(pmux) |
		// enable peripheral multiplexing
		PORT_WRCONFIG_PMUXEN(1)
	};
		
	// configure lower half pins
	if (pinmask & 0x0000FFFF) {
		// wrconfig.bit.HWSEL = 0;
		// wrconfig.bit.PINMASK = (uint16_t)(pinmask);
		// PORT->Group[port].WRCONFIG.reg = wrconfig.reg;
		wrconfig &= ~PORT_WRCONFIG_HWSEL(1);
		wrconfig &= ~PORT_WRCONFIG_PINMASK_Msk;
		wrconfig |= PORT_WRCONFIG_PINMASK((uint16_t)pinmask);
		PORT_REGS->GROUP[port].PORT_WRCONFIG = wrconfig;
	}
	// upper half
	if (pinmask & 0xFFFF0000) {
		// wrconfig.bit.HWSEL = 1;
		// wrconfig.bit.PINMASK = (uint16_t)(pinmask >> 16);
		// PORT->Group[port].WRCONFIG.reg = wrconfig.reg;
		wrconfig |= PORT_WRCONFIG_HWSEL(1);
		wrconfig &= ~PORT_WRCONFIG_PINMASK_Msk;
		wrconfig |= PORT_WRCONFIG_PINMASK((uint16_t)(pinmask >> 16));
		PORT_REGS->GROUP[port].PORT_WRCONFIG = wrconfig;
	}
}