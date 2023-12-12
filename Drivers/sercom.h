#ifndef SERCOM_H
#define SERCOM_H

#include <stdbool.h>
#include <sam.h>

bool sercom_check(sercom_registers_t* sercom);
bool sercom_init(sercom_registers_t* sercom);

#endif