#ifndef TIME_H
#define TIME_H

#include <stdint.h>

#ifndef F_CPU
#define F_CPU 48000000UL
#endif

// multiply by ticks to get time in s / ms / us
#define TIMER_US_MULTIPLIER (1E6f/F_CPU)
#define TIMER_MS_MULTIPLIER (1000.0f/F_CPU)
#define TIMER_S_MULTIPLIER (1.0f/F_CPU)

// multiply by time in s / ms / us to get ticks
#define TIMER_TICK_US_MULTIPLIER (F_CPU/1E6f)
#define TIMER_TICK_MS_MULTIPLIER (F_CPU/1000.0f)
#define TIMER_TICK_S_MULTIPLIER (F_CPU)

void set_clock_48m();

void delay_8c(uint32_t n);

#define delay_us(n) delay_8c((n)*6);
#define delay_ms(n) delay_8c((n)*6000);

void init_timer();

void start_timer();

uint32_t read_timer_20ns();

float read_timer_us();
float read_timer_ms();
float read_timer_s();


void init_timer_interrupt();
void timer_enable_interrupt();
void timer_disable_interrupt();
void timer_clear_interrupt();
void timer_set_interrupt_time(uint32_t time);

#endif