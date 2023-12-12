#ifndef RTOS_H
#define RTOS_H

#include "rtos_config.h"
#include "rtos_process.h"
#include "rtos_time.h"
#ifdef RTOS_PREEMPT
#include "rtos_preemption.h"
#endif

#define USER_STACK_BASE (RTOS_STACK_ALLOC+MAIN_STACK_ALLOC)

void SOS();

#endif