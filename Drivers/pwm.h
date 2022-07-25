#ifndef PWM_H
#define PWM_H

#include <stdbool.h>
#include "samd21.h"

bool pwm_init_out(Tcc* tcc);

void pwm_write(Tcc* tcc, int channel, float value);

#endif