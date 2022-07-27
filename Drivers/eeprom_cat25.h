#ifndef EEPROM_CAT25
#define EEPROM_CAT25

#include <stdbool.h>
#include "samd21.h"
#include "port.h"

//#define EEPROM_SERCOM SERCOM1
//#define EEPROM_SS_PORT PORT_PORTA
//#define EEPROM_SS_PIN PORT_PA19

#define EEPROM_WREN 0b00000110
#define EEPROM_WRDI 0b00000100
#define EEPROM_RDSR 0b00000101
#define EEPROM_WRSR 0b00000001
#define EEPROM_READ 0b00000011
#define EEPROM_WRITE 0b00000010

#define EEPROM_RDY_MASK 0b00000001

#define EEPROM_PAGE_SIZE 64

typedef struct __attribute__((packed)) {
	Sercom* sercom; // serial communication port
	PIN sspin; // SS pin
	uint8_t writecycletime; // time in ms of one write cycle
	uint16_t size; // size of eeprom
} EEPROM_CAT25_DESCRIPTOR;

void eeprom_cat25_init(EEPROM_CAT25_DESCRIPTOR* desc);
void eeprom_cat25_write_enable(EEPROM_CAT25_DESCRIPTOR* desc);
void eeprom_cat25_write_disable(EEPROM_CAT25_DESCRIPTOR* desc);
uint8_t eeprom_cat25_read_byte(EEPROM_CAT25_DESCRIPTOR* desc, uint32_t address);
void eeprom_cat25_write_byte(EEPROM_CAT25_DESCRIPTOR* desc, uint32_t address, uint8_t data);
void eeprom_cat25_read(EEPROM_CAT25_DESCRIPTOR* desc, uint32_t address, int count, void* writeback);
void eeprom_cat25_write(EEPROM_CAT25_DESCRIPTOR* desc, uint32_t address, int count, void* data);

#endif