#ifndef EEPROM_CAT25
#define EEPROM_CAT25

#include <stdbool.h>
#include "samd21.h"

#define EEPROM_SERCOM SERCOM1
#define EEPROM_SS_PORT PORT_PORTA
#define EEPROM_SS_PIN PORT_PA19

void eeprom_init();
uint8_t eeprom_read_byte(uint32_t address);
void eeprom_write_byte(uint32_t address, uint8_t data);
void eeprom_read(uint32_t address, int count, void* writeback);
void eeprom_write(uint32_t address, int count, void* data);

#endif