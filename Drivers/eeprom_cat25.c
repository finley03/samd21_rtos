#include "eeprom_cat25.h"
#include "port.h"
#include "spi.h"

#define EEPROM_WREN 0b00000110
#define EEPROM_WRDI 0b00000100
#define EEPROM_RDSR 0b00000101
#define EEPROM_WRSR 0b00000001
#define EEPROM_READ 0b00000011
#define EEPROM_WRITE 0b00000010

#define EEPROM_RDY_MASK 0b00000001

#define EEPROM_PAGE_SIZE 64

void eeprom_init() {
	port_set_output(EEPROM_SS_PORT, EEPROM_SS_PIN);
	port_set(EEPROM_SS_PORT, EEPROM_SS_PIN);
}

void eeprom_write_enable() {
	port_clear(EEPROM_SS_PORT, EEPROM_SS_PIN);
	spi_command(EEPROM_SERCOM, EEPROM_WREN);
	port_set(EEPROM_SS_PORT, EEPROM_SS_PIN);
}

void eeprom_write_disable() {
	port_clear(EEPROM_SS_PORT, EEPROM_SS_PIN);
	spi_command(EEPROM_SERCOM, EEPROM_WRDI);
	port_set(EEPROM_SS_PORT, EEPROM_SS_PIN);
}

uint8_t eeprom_read_status() {
	port_clear(EEPROM_SS_PORT, EEPROM_SS_PIN);
	spi_command(EEPROM_SERCOM, EEPROM_RDSR);
	uint8_t out = spi_command(EEPROM_SERCOM, 0);
	port_set(EEPROM_SS_PORT, EEPROM_SS_PIN);
	return out;
}

void eeprom_write_status(uint8_t data) {
	port_clear(EEPROM_SS_PORT, EEPROM_SS_PIN);
	spi_command(EEPROM_SERCOM, EEPROM_WRSR);
	spi_command(EEPROM_SERCOM, data);
	port_set(EEPROM_SS_PORT, EEPROM_SS_PIN);
}

uint8_t eeprom_read_byte(uint32_t address) {
	port_clear(EEPROM_SS_PORT, EEPROM_SS_PIN);
	spi_command(EEPROM_SERCOM, EEPROM_READ);
	spi_command(EEPROM_SERCOM, (uint8_t)(address >> 8));
	spi_command(EEPROM_SERCOM, (uint8_t)address);
	uint8_t out = spi_command(EEPROM_SERCOM, 0);
	port_set(EEPROM_SS_PORT, EEPROM_SS_PIN);
	return out;
}

void eeprom_write_byte(uint32_t address, uint8_t data) {
	eeprom_write_enable();
	port_clear(EEPROM_SS_PORT, EEPROM_SS_PIN);
	spi_command(EEPROM_SERCOM, EEPROM_WRITE);
	spi_command(EEPROM_SERCOM, (uint8_t)(address >> 8));
	spi_command(EEPROM_SERCOM, (uint8_t)address);
	spi_command(EEPROM_SERCOM, data);
	port_set(EEPROM_SS_PORT, EEPROM_SS_PIN);
	eeprom_write_disable();
	while (eeprom_read_status() & EEPROM_RDY_MASK);
}

void eeprom_read(uint32_t address, int count, void* writeback) {
	port_clear(EEPROM_SS_PORT, EEPROM_SS_PIN);
	spi_command(EEPROM_SERCOM, EEPROM_READ);
	spi_command(EEPROM_SERCOM, (uint8_t)(address >> 8));
	spi_command(EEPROM_SERCOM, (uint8_t)address);
	for (int i = 0; i < count; ++i) ((uint8_t*)writeback)[i] = spi_command(EEPROM_SERCOM, 0);
	port_set(EEPROM_SS_PORT, EEPROM_SS_PIN);
}

void eeprom_write(uint32_t address, int count, void* data) {
	uint32_t top = address + count - 1;
	uint32_t nr_pages = (top / EEPROM_PAGE_SIZE) - (address / EEPROM_PAGE_SIZE) + 1;
	uint32_t data_index = 0;
	eeprom_write_enable();
	for (int i = 0; i < nr_pages; ++i) {
		port_clear(EEPROM_SS_PORT, EEPROM_SS_PIN);
		spi_command(EEPROM_SERCOM, EEPROM_WRITE);
		spi_command(EEPROM_SERCOM, (uint8_t)(address >> 8));
		spi_command(EEPROM_SERCOM, (uint8_t)address);
		for (int j = address % EEPROM_PAGE_SIZE; address <= top && j < EEPROM_PAGE_SIZE; ++j) {
			spi_command(EEPROM_SERCOM, ((uint8_t*)data)[data_index++]);
			++address;
		}
		port_clear(EEPROM_SS_PORT, EEPROM_SS_PIN);
		// wait until write cycle is done
		while (eeprom_read_status() & EEPROM_RDY_MASK);
	}
	eeprom_write_disable();
}