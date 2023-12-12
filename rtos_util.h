#ifndef RTOS_UTIL_H
#define RTOS_UTIL_H

#include "rtos_config.h"

#include <stdbool.h>
#include <stdint.h>

#define ARRLEN(a) (sizeof(a)/sizeof(a[0]))

// include drivers

#ifdef TIME_DRIVER
#include TIME_DRIVER
#else
#error Mandatory driver "TIME_DRIVER" not defined
#endif

#ifdef PORT_DRIVER
#include PORT_DRIVER
#endif

// check macro definitions

#ifndef RTOS_MAX_PROCESS_COUNT
#error Mandatory value "RTOS_MAX_PROCESS_COUNT" not defined
#endif

#ifndef TIME_S_MULT
#error Mandatory value "TIME_S_MULT" not defined
#endif

#ifndef TIME_MS_MULT
#error Mandatory value "TIME_MS_MULT" not defined
#endif

#ifndef TIME_US_MULT
#error Mandatory value "TIME_US_MULT" not defined
#endif

#ifndef TIME_TICKS_S_MULT
#error Mandatory value "TIME_TICKS_S_MULT" not defined
#endif

#ifndef TIME_TICKS_MS_MULT
#error Mandatory value "TIME_TICKS_MS_MULT" not defined
#endif

#ifndef TIME_TICKS_US_MULT
#error Mandatory value "TIME_TICKS_US_MULT" not defined
#endif

#ifdef RTOS_PREEMPT

#ifndef RTOS_TICK_TIME
#error Value "RTOS_TICK_TIME" must be defined when "RTOS_PREEMPT" is defined
#endif

#endif

#ifdef MAIN_PREEMPT
#ifndef RTOS_PREEMPT
#warning Preemption is enabled for the main function, however preemption is not enabled in the kernel
#endif
#endif

// check function definitions

#ifndef delay_ms
#error Mandatory function "delay_ms" not defined
#endif

#ifndef delay_us
#error Mandatory function "delay_us" not defined
#endif

#ifndef time_init
#error Mandatory function "time_init" not defined
#endif

#ifndef time_read_ticks
#error Mandatory funcition "time_read_ticks" not defined
#endif

#undef DEBUG_LED
#if defined(led_init) || defined(led_on) || defined(led_off) || defined(led_toggle)
#if defined(led_init) && defined(led_on) && defined(led_off) && defined(led_toggle)
#define DEBUG_LED
#else
#error One or more "led_xxx" functions defined, but not all
#endif
#endif

#ifdef RTOS_PREEMPT

#ifndef preempt_init_interrupts
#error Function "preempt enable interrupts" must be defined when "RTOS_PREEMPT" is defined
#endif

#ifndef preempt_enable_interrupts
#error Function "preempt_enable_interrupts" must be defined when "RTOS_PREEMPT" is defined
#endif

#ifndef preempt_disable_interrupts
#error Function "preempt_disable_interrupts" must be defined when "RTOS_PREEMPT" is defined
#endif

#ifndef preempt_clear_interrupts
#error Function "preempt_clear_interrupts" must be defined when "RTOS_PREEMPT" is defined
#endif

#ifndef preempt_set_interrupt_time
#error Function "preempt_set_interrupt_time" must be defined when "RTOS_PREEMPT" is defined
#endif

#endif


#define DISCARD_REGISTERS __asm("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12")
#define READ_PROGRAM_COUNTER(pc) __asm("mov %0, pc" : "=r" (pc))
//#define READ_LINI(pc) __asm("mov %0, pc" : "=r" (pc))
#define POP_PROGRAM_COUNTER() __asm("pop {pc}")
#define USE_MSP() __set_CONTROL(0);
#define USE_PSP() __set_CONTROL(2);
	
#define rtos_preemption_handler TC4_Handler


extern const int time_ticks_s_mult;
extern const int time_ticks_ms_mult;
extern const int time_ticks_us_mult;

// register access
volatile register unsigned stack_pointer __asm("sp");
volatile register unsigned link_register __asm("lr");

// extern uint32_t rtos_stack_pointer;

//void dummy_function();

#endif