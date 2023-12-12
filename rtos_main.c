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
	init_process(mainproc, MAINFUNC, RTOS_STACK_ALLOC, MAIN_STACK_ALLOC);
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
	
	// initialize preemption
	#ifdef RTOS_PREEMPT
	preempt_init_interrupts();
	#endif
	
	return true;
}

#ifdef DEBUG_LED
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
	__disable_irq();
	
	while(1) {
		morse("...---...");
		delay_ms(600);
	}
}
#else
void SOS() {
	__disable_irq();
	
	while(1);
}
#endif