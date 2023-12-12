// config header file for RTOS
// definitions for includes are here

#ifndef RTOS_CONFIG_H
#define RTOS_CONFIG_H

//----------SETTINGS----------//

// RTOS_MAX_PROCESS_COUNT - maximum number of processes the RTOS needs
// to be able to handle. Don't set to an arbitrarily high value, each
// extra process supported consumes an extra 4 bytes of ram.
#define RTOS_MAX_PROCESS_COUNT 16

// define the main function
#define MAINFUNC main

// ram allocated to the OS
// remember any callbacks and interrupts must fit in this space
#define RTOS_STACK_ALLOC 0x300

// ram allocated to the main function
#define MAIN_STACK_ALLOC 0x400

// enable preemptive multitasking
#define RTOS_PREEMPT

// enable preemption by default on the main function
//#define MAIN_PREEMPT

// RTOS tick time for preemption (microseconds)
#define RTOS_TICK_TIME 1000

//----------HEADERS----------//

// Headers to provide information for this file

#include <sam.h>

//----------PORT DEFINITIONS----------//

#define DEBUG_LED_PORT PORT_PORTB
#define DEBUG_LED_MASK PORT_PB22

//----------DRIVERS----------//

// the following are definitions of device drivers

// MANDATORY DRIVERS
// required for the OS to be able to compile and run

// time driver used for delay functions
#define TIME_DRIVER "Drivers/time.h"

// OPTIONAL DRIVERS
// other drivers the OS will use if they are defined

// port driver will be used for default 
#define PORT_DRIVER "Drivers/port.h"

// OTHER DRIVERS
// for programs or other functionality


//----------VALUES---------//

// constant values to be used by the OS

// MANDATORY VALUES

// TIME_S_MULT - multiplier to convert ticks to seconds
#define TIME_S_MULT TIMER_S_MULTIPLIER

// TIME_MS_MULT - multiplier to convert ticks to milliseconds
#define TIME_MS_MULT TIMER_MS_MULTIPLIER

// TIME_US_MULT - multiplier to convert ticks to microseconds
#define TIME_US_MULT TIMER_US_MULTIPLIER

// TIME_TICKS_S_MULT - multiplier to convert seconds to ticks
#define TIME_TICKS_S_MULT TIMER_TICK_S_MULTIPLIER

// TIME_TICKS_MS_MULT - multiplier to convert milliseconds to ticks
#define TIME_TICKS_MS_MULT TIMER_TICK_MS_MULTIPLIER

// TIME_TICKS_US_MULT - multiplier to convert microseconds to ticks
#define TIME_TICKS_US_MULT TIMER_TICK_US_MULTIPLIER

//----------FUNCTIONS----------//

// the following are functions the OS can use
// define the names where possible

// MANDATORY FUNCTIONS
// must be defined for OS to compile

// void delay_ms(int time)
//#define delay_ms

// void delay_us(int time)
//#define delay_ms

// void time_init()
#define time_init init_timer

// uint32_t time_read_ticks()
#define time_read_ticks read_timer_20ns

// If RTOS_PREEMPT is defined, these functions are mandatory

// void time_init_interrupts()
#define preempt_init_interrupts init_timer_interrupt

// void preempt_enable_interrupts()
#define preempt_enable_interrupts timer_enable_interrupt

// void preempt_disable_interrupts()
#define preempt_disable_interrupts timer_disable_interrupt

// void preempt_clear_interrupts()
#define preempt_clear_interrupts timer_clear_interrupt

// void preempt_set_interrupt_time(uint32_t time)
#define preempt_set_interrupt_time timer_set_interrupt_time

// OPTIONAL_FUNCTIONS
// these will be used if they are defined

//---led block---// all must be defined

// void led_init()
#define led_init() port_set_output(DEBUG_LED_PORT, DEBUG_LED_MASK)

// void led_on()
#define led_on() port_set(DEBUG_LED_PORT, DEBUG_LED_MASK)

// void led_off()
#define led_off() port_clear(DEBUG_LED_PORT, DEBUG_LED_MASK)

// void led_toggle()
#define led_toggle() port_toggle(DEBUG_LED_PORT, DEBUG_LED_MASK)

//---end block---//


// configure_clock(void)
#define configure_clock set_clock_48m

//----------END OF CONFIG----------//

#endif