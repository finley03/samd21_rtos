#include "port.h"
#include "samd21.h"

void port_set_output(uint8_t port, uint32_t pinmask) {
	PORT->Group[port].DIRSET.reg = pinmask;
}

void port_set_input(uint8_t port, uint32_t pinmask) {
	PORT->Group[port].DIRCLR.reg = pinmask;
}

void port_enable_input(uint8_t port, uint32_t pinmask) {
	for (int i = 0; i < 32; ++i) {
		if ((1 << i) & pinmask) PORT->Group[port].PINCFG[i].bit.INEN = 1;
	}
}

void port_disable_input(uint8_t port, uint32_t pinmask) {
	for (int i = 0; i < 32; ++i) {
		if ((1 << i) & pinmask) PORT->Group[port].PINCFG[i].bit.INEN = 0;
	}
}


void port_set(uint8_t port, uint32_t pinmask) {
	PORT->Group[port].OUTSET.reg = pinmask;
}

void port_clear(uint8_t port, uint32_t pinmask) {
	PORT->Group[port].OUTCLR.reg = pinmask;
}

void port_toggle(uint8_t port, uint32_t pinmask) {
	PORT->Group[port].OUTTGL.reg = pinmask;
}


void port_wrconfig(uint8_t port, uint32_t pmux, uint32_t pinmask) {
	PORT_WRCONFIG_Type wrconfig = {
		// enable writing to pincfg
		.bit.WRPINCFG = 1,
		// enable writing to pmux
		.bit.WRPMUX = 1,
		// select pmux function
		.bit.PMUX = pmux,
		// enable peripheral multiplexing
		.bit.PMUXEN = 1
	};
		
	// configure lower half pins
	if (pinmask & 0x0000FFFF) {
		wrconfig.bit.HWSEL = 0;
		wrconfig.bit.PINMASK = (uint16_t)(pinmask);
		PORT->Group[port].WRCONFIG.reg = wrconfig.reg;
	}
	// upper half
	if (pinmask & 0xFFFF0000) {
		wrconfig.bit.HWSEL = 1;
		wrconfig.bit.PINMASK = (uint16_t)(pinmask >> 16);
		PORT->Group[port].WRCONFIG.reg = wrconfig.reg;
	}
}