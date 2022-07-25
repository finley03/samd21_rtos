#include "spi.h"
#include "sercom.h"
#include "time.h"

bool spi_init(Sercom* sercom, uint8_t cpol, uint8_t cpha, uint8_t dipo, uint8_t dopo, uint32_t baud) {
	if (!sercom_init(sercom)) return false;
	
	// set mode to master
	sercom->SPI.CTRLA.reg = ((uint32_t)cpol << SERCOM_SPI_CTRLA_CPOL_Pos) | ((uint32_t)cpha << SERCOM_SPI_CTRLA_CPHA_Pos) | 
		SERCOM_SPI_CTRLA_MODE_SPI_MASTER | ((uint32_t)dipo << SERCOM_SPI_CTRLA_DIPO_Pos) | 
		((uint32_t)dopo << SERCOM_SPI_CTRLA_DOPO_Pos);
		
	// enable rx
	sercom->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;
	
	spi_set_baud(sercom, baud);
	
	// enable SPI
	sercom->SPI.CTRLA.bit.ENABLE = 1;
	
	// wait for sync
	while (sercom->SPI.SYNCBUSY.bit.ENABLE);
	return true;
}

bool spi_set_baud(Sercom* sercom, uint32_t baud) {
	if (!sercom_check(sercom)) return false;
	
	float baudval = (float)F_CPU / (2 * baud) - 1;
	sercom->SPI.BAUD.reg = (uint8_t)baudval;
	return true;
}

uint8_t spi_command(Sercom* sercom, uint8_t data) {
	// wait until ready to send
	while (!sercom->SPI.INTFLAG.bit.DRE);
	sercom->SPI.DATA.reg = data;
	// wait until done
	while (!sercom->SPI.INTFLAG.bit.TXC);
	// read buffer
	return sercom->SPI.DATA.reg;
}