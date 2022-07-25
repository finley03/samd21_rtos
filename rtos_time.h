#ifndef RTOS_TIME_H
#define RTOS_TIME_H

#include <stdint.h>

extern const int time_ticks_s_mult;
extern const int time_ticks_ms_mult;
extern const int time_ticks_us_mult;

//----------rtos_delay_ms : function----------//
// Function: Stop process execution for AT LEAST the time specified.
// Control is handed to the OS in this time and other processes
// will execute. The OS will attempt to return control to the process
// after the specified delay, or as soon as possible after if this
// is not possible.
// Thread safety: Thread safe for any process
//void rtos_delay_ms(uint32_t n);

//----------rtos_delay_us : function----------//
// Function: Stop process execution for AT LEAST the time specified.
// Control is handed to the OS in this time and other processes
// will execute. The OS will attempt to return control to the process
// after the specified delay, or as soon as possible after if this
// is not possible.
// Thread safety: Thread safe for any process
//void rtos_delay_us(uint32_t n);

//----------rtos_delay_ctick : function----------//
// Function: Stop process execution for at least the time specified
// in timer ticks. Control is handed to the OS in this time and
// other processes will execute. The OS will attempt to return
// control to the process after the specified delay, or as soon as
// possible after.
// Thread safety: Thread safe for any process
void rtos_delay_ctick(uint32_t n);

//----------rtos_delay_s : macro----------//
// Function: alias to the function rtos_delay_ctick
// Includes a multiplier that converts seconds to timer ticks.
// Thread safety: Thread safe for any process
#define rtos_delay_s(n) rtos_delay_ctick((n)*time_ticks_s_mult)

//----------rtos_delay_ms : macro----------//
// Function: alias to the function rtos_delay_ctick
// Includes a multiplier that converts milliseconds to timer ticks.
// Thread safety: Thread safe for any process
#define rtos_delay_ms(n) rtos_delay_ctick((n)*time_ticks_ms_mult)

//----------rtos_delay_s : macro----------//
// Function: alias to the function rtos_delay_ctick
// Includes a multiplier that converts microseconds to timer ticks.
// Thread safety: Thread safe for any process
#define rtos_delay_us(n) rtos_delay_ctick((n)*time_ticks_us_mult)

#endif