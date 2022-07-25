#ifndef EEPROM_CAT25
#define EEPROM_CAT25

#include <stdbool.h>
#include "samd21.h"

#define EEPROM_SERCOM SERCOM1
#define EEPROM_SS_PORT PORT_PORTA
//#define EEPROM_SS_PIN 

bool eeprom_init();

#endif