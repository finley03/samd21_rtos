#include "eeprom_cat25.h"
#include "port.h"
#include "spi.h"

void eeprom_cat25_init(EEPROM_CAT25_DESCRIPTOR* desc) {
	port_set_output(desc->sspin.port, desc->sspin.pin);
	port_set(desc->sspin.port, desc->sspin.pin);
}

void eeprom_cat25_write_enable(EEPROM_CAT25_DESCRIPTOR* desc) {
	port_clear(desc->sspin.port, desc->sspin.pin);
	spi_command(desc->sercom, EEPROM_WREN);
	port_set(desc->sspin.port, desc->sspin.pin);
}

void eeprom_cat25_write_disable(EEPROM_CAT25_DESCRIPTOR* desc) {
	port_clear(desc->sspin.port, desc->sspin.pin);
	spi_command(desc->sercom, EEPROM_WRDI);
	port_set(desc->sspin.port, desc->sspin.pin);
}

uint8_t eeprom_cat25_read_status(EEPROM_CAT25_DESCRIPTOR* desc) {
	port_clear(desc->sspin.port, desc->sspin.pin);
	spi_command(desc->sercom, EEPROM_RDSR);
	uint8_t out = spi_command(desc->sercom, 0);
	port_set(desc->sspin.port, desc->sspin.pin);
	return out;
}

void eeprom_cat25_write_status(EEPROM_CAT25_DESCRIPTOR* desc, uint8_t data) {
	port_clear(desc->sspin.port, desc->sspin.pin);
	spi_command(desc->sercom, EEPROM_WRSR);
	spi_command(desc->sercom, data);
	port_set(desc->sspin.port, desc->sspin.pin);
}

uint8_t eeprom_cat25_read_byte(EEPROM_CAT25_DESCRIPTOR* desc, uint32_t address) {
	port_clear(desc->sspin.port, desc->sspin.pin);
	spi_command(desc->sercom, EEPROM_READ);
	spi_command(desc->sercom, (uint8_t)(address >> 8));
	spi_command(desc->sercom, (uint8_t)address);
	uint8_t out = spi_command(desc->sercom, 0);
	port_set(desc->sspin.port, desc->sspin.pin);
	return out;
}

void eeprom_cat25_write_byte(EEPROM_CAT25_DESCRIPTOR* desc, uint32_t address, uint8_t data) {
	eeprom_cat25_write_enable(desc);
	port_clear(desc->sspin.port, desc->sspin.pin);
	spi_command(desc->sercom, EEPROM_WRITE);
	spi_command(desc->sercom, (uint8_t)(address >> 8));
	spi_command(desc->sercom, (uint8_t)address);
	spi_command(desc->sercom, data);
	port_set(desc->sspin.port, desc->sspin.pin);
	eeprom_cat25_write_disable(desc);
	while (eeprom_cat25_read_status(desc) & EEPROM_RDY_MASK);
}

void eeprom_cat25_read(EEPROM_CAT25_DESCRIPTOR* desc, uint32_t address, int count, void* writeback) {
	port_clear(desc->sspin.port, desc->sspin.pin);
	spi_command(desc->sercom, EEPROM_READ);
	spi_command(desc->sercom, (uint8_t)(address >> 8));
	spi_command(desc->sercom, (uint8_t)address);
	for (int i = 0; i < count; ++i) ((uint8_t*)writeback)[i] = spi_command(desc->sercom, 0);
	port_set(desc->sspin.port, desc->sspin.pin);
}

void eeprom_cat25_write(EEPROM_CAT25_DESCRIPTOR* desc, uint32_t address, int count, void* data) {
	uint32_t top = address + count - 1;
	uint32_t nr_pages = (top / EEPROM_PAGE_SIZE) - (address / EEPROM_PAGE_SIZE) + 1;
	uint32_t data_index = 0;
	eeprom_cat25_write_enable(desc);
	for (int i = 0; i < nr_pages; ++i) {
		port_clear(desc->sspin.port, desc->sspin.pin);
		spi_command(desc->sercom, EEPROM_WRITE);
		spi_command(desc->sercom, (uint8_t)(address >> 8));
		spi_command(desc->sercom, (uint8_t)address);
		for (int j = address % EEPROM_PAGE_SIZE; address <= top && j < EEPROM_PAGE_SIZE; ++j) {
			spi_command(desc->sercom, ((uint8_t*)data)[data_index++]);
			++address;
		}
		port_set(desc->sspin.port, desc->sspin.pin);
		// wait until write cycle is done
		while (eeprom_cat25_read_status(desc) & EEPROM_RDY_MASK);
	}
	eeprom_cat25_write_disable(desc);
}