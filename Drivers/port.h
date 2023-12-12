#ifndef PORT_H
#define PORT_H

#include <stdint.h>

#define PORT_COUNT 2

#define PORT_PORTA 0
#define PORT_PORTB 1

#define PORT_PMUX_A 0x0
#define PORT_PMUX_B 0x1
#define PORT_PMUX_C 0x2
#define PORT_PMUX_D 0x3
#define PORT_PMUX_E 0x4
#define PORT_PMUX_F 0x5
#define PORT_PMUX_G 0x6
#define PORT_PMUX_H 0x7
#define PORT_PMUX_I 0x8

typedef struct __attribute__((packed)) {
	uint32_t pin;
	uint8_t port;
} Pin;

void port_set_output(uint8_t port, uint32_t pinmask);
void port_set_input(uint8_t port, uint32_t pinmask);
void port_enable_input(uint8_t port, uint32_t pinmask);
void port_disable_input(uint8_t port, uint32_t pinmask);

void port_set(uint8_t port, uint32_t pinmask);
void port_clear(uint8_t port, uint32_t pinmask);
void port_toggle(uint8_t port, uint32_t pinmask);

void port_wrconfig(uint8_t port, uint32_t pmux, uint32_t pinmask);

#endif