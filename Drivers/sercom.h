#ifndef SERCOM_H
#define SERCOM_H

#include <stdbool.h>
#include "samd21.h"

bool sercom_check(Sercom* sercom);
bool sercom_init(Sercom* sercom);

#endif