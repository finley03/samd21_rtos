#include "rtos_time.h"
#include "rtos_util.h"
#include "rtos_process.h"

extern void yield_process(int process_status);

void rtos_delay_callback() {
	// if ((int)(current_process->return_deadline - time_read_ticks()) <= 0) current_process->status = Process_State_Running;
	volatile uint32_t time = time_read_ticks();
	int t = (int)(current_process->return_deadline - time);
	static volatile int min = INT32_MAX;
	min = (t < min) ? t : min;
	if (t <= 0) current_process->status = Process_State_Running;
}

void rtos_delay_ctick(uint32_t n) {
	// set return deadline for process
	current_process->return_deadline = time_read_ticks() + n;
	
	// create structure to define when process resumes execution
	volatile Process_Wait_Until_Data data = {
		.variable = 0,
		.value = 0,
		.mask = 0,
		.condition = Process_Wait_Until_None,
		.callback = rtos_delay_callback
	};
	
	// make internal data pointer point to structure
	current_process->internal_data = &data;
	
	yield_process(Process_State_Blocked);
}