#include "rtos_time.h"
#include "rtos_util.h"
#include "rtos_process.h"


extern void yield_process(int process_status);


//void rtos_delay_ms(uint32_t n) {
	//// set return deadline for process
	//current_process->return_deadline = time_read_ticks() + (n * time_ticks_ms_mult);
	//
	//// indicate process has not ended
	//current_process->status = Process_State_Running;
	//
	//// set stack pointer
	//current_process->stack_pointer = stack_pointer;
	//
	//// set stack pointer to be above first address in stack (link register)
	//stack_pointer = current_process->stack_base - 4;
	//
	//// read program counter
	//uint32_t pc;
	//READ_PROGRAM_COUNTER(pc);
	//current_process->program_counter = pc + 4;
	//
	//// branch (return) to process caller
	//POP_PROGRAM_COUNTER();
	//
	//// execution will resume here when control is returned by the OS
	//DISCARD_REGISTERS;
	//
	//// set ended to true so OS will know if process ends
	//current_process->status = Process_State_Done;
	//
	//// wait until execution should continue
	//while((int)(current_process->return_deadline - time_read_ticks()) > 0);
//}

//void rtos_delay_ms(uint32_t n) {
	//// set return deadline for process
	//current_process->return_deadline = time_read_ticks() + (n * time_ticks_ms_mult);
	//
	//yield_process(Process_State_Running);
	//
	//// wait until execution should continue
	//while((int)(current_process->return_deadline - time_read_ticks()) > 0);
//}

//void rtos_delay_us(uint32_t n) {
	//// set return deadline for process
	//current_process->return_deadline = time_read_ticks() + (n * time_ticks_us_mult);
	//
	//// indicate process has not ended
	//current_process->status = Process_State_Running;
	//
	//// set stack pointer
	//current_process->stack_pointer = stack_pointer;
	//
	//// set stack pointer to be above first address in stack (link register)
	//stack_pointer = current_process->stack_base - 4;
	//
	//// read program counter
	//uint32_t pc;
	//READ_PROGRAM_COUNTER(pc);
	//current_process->program_counter = pc + 4;
	//
	//// branch (return) to process caller
	//POP_PROGRAM_COUNTER();
	//
	//// execution will resume here when control is returned by the OS
	//DISCARD_REGISTERS;
	//
	//// set ended to true so OS will know if process ends
	//current_process->status = Process_State_Done;
	//
	//// wait until execution should continue
	//while((int)(current_process->return_deadline - time_read_ticks()) > 0);
//}

//void rtos_delay_us(uint32_t n) {
	//// set return deadline for process
	//current_process->return_deadline = time_read_ticks() + (n * time_ticks_us_mult);
	//
	//yield_process(Process_State_Running);
	//
	//// wait until execution should continue
	//while((int)(current_process->return_deadline - time_read_ticks()) > 0);
//}


//void rtos_delay_ctick(uint32_t n) {
	//// set return deadline for process
	//current_process->return_deadline = time_read_ticks() + n;
	//
	//// yield to os
	//yield_process(Process_State_Running);
	//
	//// wait until execution should continue
	//while((int)(current_process->return_deadline - time_read_ticks()) > 0);
//}

//void rtos_delay_ctick(uint32_t n) {
	//// set return deadline for process
	//current_process->return_deadline = time_read_ticks() + n;
	//
	//// create structure to define when process resumes execution
	//volatile Process_Wait_Until_Data data = {
		//.variable = 0,
		//.value = 0,
		//.mask = 0,
		//.condition = Process_Wait_Until_Deadline
	//};
	//
	//// make internal data pointer point to structure
	//current_process->internal_data = &data;
	//
	//yield_process(Process_State_Blocked);
//}

void rtos_delay_callback() {
	if ((int)(current_process->return_deadline - time_read_ticks()) <= 0) current_process->status = Process_State_Running;
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