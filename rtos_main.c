#include "rtos_util.h"
#include "rtos_process.h"
#include "rtos_time.h"

Process _mainproc;
Process* mainproc;
extern int process_count;

extern void switch_process(Process* process);
extern void init_process_queue();
extern Process* next_process();

extern void MAINFUNC();

bool rtos_init();
void SOS();


int rtos_main(void) {
	rtos_init();
	
	mainproc = &_mainproc;
	init_process(mainproc, MAINFUNC, RTOS_RAM_ALLOC, MAIN_RAM_ALLOC);
	#if defined(RTOS_PREEMPT) && defined(MAIN_PREEMPT)
	mainproc->enable_preempt = true;
	#endif
	
	dispatch_process(mainproc);
	
	// repeat while there are running processes
	Process* lastproc;
	while (process_count) {
		if (!(lastproc = next_process())) break;
		dispatch_process(lastproc);
	}
	
	#ifdef DEBUG_LED
	if (process_count) SOS();
	led_on();
	#endif
	
	return 0;
}


bool rtos_init() {
	#ifdef configure_clock
	configure_clock();
	#endif
	
	time_init();
	
	#ifdef DEBUG_LED
	led_init();
	#endif
	
	init_process_queue();
	
	//init_timer_interrupt();
	//timer_set_interrupt_time(time_read_ticks() + time_ticks_s_mult);
	//timer_clear_interrupt();
	//timer_enable_interrupt();
	
	// initialize preemption
	#ifdef RTOS_PREEMPT
	preempt_init_interrupts();
	#endif
	
	return true;
}

void morse(const char* string) {
	char* c = string;
	while (*c != '\0') {
		int delay = 0;
		if (*c == '.') delay = 100;
		else if (*c == '-') delay = 200;
		led_on();
		delay_ms(delay);
		led_off();
		delay_ms(100);
		++c;
	}
}

void SOS() {
	while(1) {
		morse("...---...");
		//led_toggle();
		delay_ms(600);
	}
}