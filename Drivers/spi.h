#ifndef SPI_H
#define SPI_H

#include <stdbool.h>
#include <sam.h>

// the given SERCOM is validated
bool spi_init(sercom_registers_t* sercom, uint8_t cpol, uint8_t cpha, uint8_t dipo, uint8_t dopo, uint32_t baud);
bool spi_set_baud(sercom_registers_t* sercom, uint32_t baud);
// for these the SERCOM is not validated
uint8_t spi_command(sercom_registers_t* sercom, uint8_t data);
void spi_flush(sercom_registers_t* sercom);

#endif