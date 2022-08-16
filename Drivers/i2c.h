#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include "samd21.h"

bool i2c_init(Sercom* sercom);

void i2c_send_data(Sercom* sercom, uint8_t device_address, uint8_t* data, int count);
void i2c_receive_data(Sercom* sercom, uint8_t device_address, uint8_t* data, int count);

#endif