#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include "samd21.h"

// the given SERCOM is validated
// NOTE only 0 and 2 are valid for txpad, 0 to 3 are valid for rxpad
bool uart_init(Sercom* sercom, uint32_t txpad, uint32_t rxpad, uint32_t baud);
bool uart_set_baud(Sercom* sercom, uint32_t baud);
// for these the SERCOM is not validated
void uart_flush(Sercom* sercom);
void uart_send_byte(Sercom* sercom, uint8_t data);
void uart_stream(Sercom* sercom, uint8_t* address, uint32_t count);
void uart_print(Sercom* sercom, char* address);
bool uart_read_byte(Sercom* sercom, uint8_t* address, uint32_t timeout);
bool uart_read(Sercom* sercom, uint8_t* address, uint32_t count, uint32_t timeout);
bool uart_check_rx_data(Sercom* sercom);

#endif