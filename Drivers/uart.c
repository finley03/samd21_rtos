#include "uart.h"
#include "sercom.h"
#include "time.h"

bool uart_init(Sercom* sercom, uint32_t txpad, uint32_t rxpad, uint32_t baud) {
	// init SERCOM peripheral
	if (!sercom_init(sercom)) return false;
	if (txpad != 0 && txpad != 2) return false;
	if (rxpad > 3) return false;
	if (txpad) txpad = 1;
	
	// init sercom usart
	// set to LSB first, internal clock
	sercom->USART.CTRLA.reg = SERCOM_USART_CTRLA_DORD | (rxpad << SERCOM_USART_CTRLA_RXPO_Pos) |
	(txpad << SERCOM_USART_CTRLA_TXPO_Pos) | SERCOM_USART_CTRLA_MODE_USART_INT_CLK;
	
	// enable tx and rx
	sercom->USART.CTRLB.reg = SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN;
	
	// wait for sync
	while (sercom->USART.SYNCBUSY.bit.CTRLB);
	
	uart_set_baud(sercom, baud);
	
	// enable USART
	sercom->USART.CTRLA.bit.ENABLE = 1;
	
	// wait for sync
	while(sercom->USART.SYNCBUSY.bit.ENABLE);
	
	return true;
}

bool uart_set_baud(Sercom* sercom, uint32_t baud) {
	if (!sercom_check(sercom)) return false;
	
	float baudval = (1.0f - ((float)baud / F_CPU * 16)) * 65536;
	sercom->USART.BAUD.reg = (uint16_t)baudval;
	return true;
}

void uart_flush(Sercom* sercom) {
	// while there is unread data touch the data register
	while (sercom->USART.INTFLAG.bit.RXC) sercom->USART.DATA.reg;
}

void uart_send_byte(Sercom* sercom, uint8_t data) {
	// wait until ready to send data
	while (!sercom->USART.INTFLAG.bit.DRE);
	sercom->USART.DATA.reg = data;
}

void uart_stream(Sercom* sercom, uint8_t* address, uint32_t count) {
	for (int i = 0; i < count; ++i) {
		while (!sercom->USART.INTFLAG.bit.DRE);
		sercom->USART.DATA.reg = address[i];
	}
}

void uart_print(Sercom* sercom, char* address) {
	while (*address != '\0') {
		while (!sercom->USART.INTFLAG.bit.DRE);
		sercom->USART.DATA.reg = *address;
		++address;
	}
}

bool uart_read_byte(Sercom* sercom, uint8_t* address, uint32_t timeout) {
	uint32_t byte_start_time = read_timer_20ns();
	// wait until data available
	while (!sercom->USART.INTFLAG.bit.RXC) {
		// check if connection has timed out
		if (read_timer_20ns() - byte_start_time > timeout) return false;
	}
	*address = (uint8_t)(sercom->USART.DATA.reg);
	return true;
}

bool uart_read(Sercom* sercom, uint8_t* address, uint32_t count, uint32_t timeout) {
	for (int i = 0; i < count; ++i) {
		uint32_t byte_start_time = read_timer_20ns();
		// wait until data available
		while (!sercom->USART.INTFLAG.bit.RXC) {
			// check if connection has timed out
			if (read_timer_20ns() - byte_start_time > timeout) return false;
		}
		address[i] = (uint8_t)(sercom->USART.DATA.reg);
	}
	return true;
}

bool uart_check_rx_data(Sercom* sercom) {
	if (sercom->USART.INTFLAG.bit.RXC) return true;
	else return false;
}