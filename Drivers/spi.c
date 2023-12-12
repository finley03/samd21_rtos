#include "spi.h"
#include "sercom.h"
#include "time.h"

bool spi_init(sercom_registers_t* sercom, uint8_t cpol, uint8_t cpha, uint8_t dipo, uint8_t dopo, uint32_t baud) {
	if (!sercom_init(sercom)) return false;
	
	// set mode to master
	sercom->SPIM.SERCOM_CTRLA = ((uint32_t)cpol << SERCOM_SPIM_CTRLA_CPOL_Pos) | ((uint32_t)cpha << SERCOM_SPIM_CTRLA_CPHA_Pos) | 
		SERCOM_SPIM_CTRLA_MODE_SPI_MASTER | ((uint32_t)dipo << SERCOM_SPIM_CTRLA_DIPO_Pos) | 
		((uint32_t)dopo << SERCOM_SPIM_CTRLA_DOPO_Pos);
		
	// enable rx
	sercom->SPIM.SERCOM_CTRLB = SERCOM_SPIM_CTRLB_RXEN(1);
	
	spi_set_baud(sercom, baud);
	
	// enable SPI
	sercom->SPIM.SERCOM_CTRLA |= SERCOM_SPIM_CTRLA_ENABLE(1);
	
	// wait for sync
	while (sercom->SPIM.SERCOM_SYNCBUSY & SERCOM_SPIM_SYNCBUSY_ENABLE_Msk);
	return true;
}

bool spi_set_baud(sercom_registers_t* sercom, uint32_t baud) {
	if (!sercom_check(sercom)) return false;
	
	float baudval = (float)F_CPU / (2 * baud) - 1;
	sercom->SPIM.SERCOM_BAUD = (uint8_t)baudval;
	return true;
}

uint8_t spi_command(sercom_registers_t* sercom, uint8_t data) {
	// wait until ready to send
	while (!(sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_DRE_Msk));
	sercom->SPIM.SERCOM_DATA = data;
	// wait until done
	while (!(sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_TXC_Msk));
	// read buffer
	return sercom->SPIM.SERCOM_DATA;
}

void spi_flush(sercom_registers_t* sercom) {
	// while there is unread data touch the data register
	while (sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_RXC_Msk) sercom->SPIM.SERCOM_DATA;
}