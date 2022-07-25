#include "rtos_process.h"
#include "rtos_util.h"

extern uint32_t _estack;
#define rtos_stack_base ((uint32_t)(&_estack))

Process* process_queue[RTOS_MAX_PROCESS_COUNT];
int process_queue_head;
int process_queue_tail;
int process_count;
Process* current_process;


void init_process(Process* process, void (*procFunction)(void), uint32_t stack_position, uint32_t stack_size) {
	process->stack_base = rtos_stack_base - stack_position;
	process->stack_size = stack_size;
	process->program_counter = (uint32_t)procFunction - 1;
	process->stack_pointer = process->stack_base;
	process->procFunction = procFunction;
	process->return_deadline = 0;
	process->status = Process_State_Ready;
	#ifdef RTOS_PREEMPT
	process->enable_preempt = false;
	#endif
}

void reset_process(Process* process) {
	process->stack_pointer = process->stack_base;
	process->program_counter = (uint32_t)process->procFunction - 1;
	process->return_deadline = 0;
	process->status = Process_State_Ready;
}

void switch_process(Process* process) {
	#ifdef RTOS_PREEMPT
	if (process->enable_preempt) {
		preempt_set_interrupt_time(time_read_ticks() + RTOS_TICK_TIME * time_ticks_us_mult);
		preempt_clear_interrupts();
		preempt_enable_interrupts();
	}
	#endif
	
	// set current process
	current_process = process;
	
	// set process state to done. If it returns and it is not finished,
	// the preemption routines will set it to running
	process->status = Process_State_Done;
	
	// save stack pointer
	rtos_stack_pointer = stack_pointer;
	
	// set stack pointer to process stack pointer
	stack_pointer = process->stack_pointer;
	
	// jump to process
	((void (*)(void))(process->program_counter + 1))();
	
	// invalidate all registers
	DISCARD_REGISTERS;
	
	// restore stack pointer
	stack_pointer = rtos_stack_pointer;
	
	#ifdef RTOS_PREEMPT
	if (process->enable_preempt) {	
		preempt_disable_interrupts();
	}
	#endif
}


void init_process_queue() {
	process_queue_head = process_queue_tail = 0;
	current_process = 0;
	process_count = 0;
}

bool dispatch_process(Process* process) {
	// check process is not finished
	if (process->status == Process_State_Done) {
		return false;
	}
	// check if there is space in the queue
	// if not return false
	if (process_count >= RTOS_MAX_PROCESS_COUNT) return false;
	// else
	
	// insert element to correct position for its deadline
	uint32_t current_time = time_read_ticks();
	// calculate time to deadline for new process
	uint32_t time_to_deadline = (process->status == Process_State_Ready) ? 0 : process->return_deadline - current_time;
	
	// loop through elements from the tail to the head of the queue
	int queue_pointer = process_queue_tail;
	
	for (int i = 0; i < process_count; ++i) {
		int next_queue_pointer = queue_pointer - 1;
		if (next_queue_pointer < 0) next_queue_pointer += RTOS_MAX_PROCESS_COUNT;
		// time to deadline of process being checked
		uint32_t next_time_to_deadline = (process_queue[next_queue_pointer]->status == Process_State_Ready) ? 0 : process_queue[next_queue_pointer]->return_deadline - current_time;
		// if deadline for new process is nearer than deadline of process being checked move the process being checked
		if ((int)next_time_to_deadline > (int)time_to_deadline) {
			process_queue[queue_pointer] = process_queue[next_queue_pointer];
		}
		else {
			break;
		}
		
		//--queue_pointer;
		//if (queue_pointer < 0) queue_pointer += process_queue_length;
		queue_pointer = next_queue_pointer;
	}
	
	process_queue[queue_pointer] = process;
	
	++process_count;
	++process_queue_tail;
	process_queue_tail %= RTOS_MAX_PROCESS_COUNT;
	
	return true;
}

Process* next_process() {
	//Process* proc = process_queue[process_queue_head];
	// find first unblocked process, or unblock process if possible
	int queue_pointer = process_queue_head;
	Process* proc = 0;
	int blocked_count = 0;
	
	for (; blocked_count < process_count; ++blocked_count) {
		proc = process_queue[queue_pointer];
		// break if not blocked
		if (proc->status != Process_State_Blocked) break;
		else {
			// attempt to unblock process
			volatile Process_Wait_Until_Data* data = proc->internal_data;
			volatile bool unblocked = false;
			uint32_t variable;
			// get variable, avoiding accessing protected portions of memory
			if (data->mask) {
				if ((data->mask & U8_MASK) == data->mask) variable = *((uint8_t*)(data->variable));
				else if ((data->mask & U16_MASK) == data->mask) variable = *((uint16_t*)(data->variable));
				else variable = *(data->variable);
				variable &= data->mask;
			}
			// switch condition
			switch (data->condition) {
				case Process_Wait_Until_Equal:
				if (variable == data->value) unblocked = true;
				break;
				case Process_Wait_Until_NotEqual:
				if (variable != data->value) unblocked = true;
				break;
				case Process_Wait_Until_Greater:
				if (variable > data->value) unblocked = true;
				break;
				case Process_Wait_Until_GEqual:
				if (variable >= data->value) unblocked = true;
				break;
				case Process_Wait_Until_Less:
				if (variable < data->value) unblocked = true;
				break;
				case Process_Wait_Until_LEqual:
				if (variable <= data->value) unblocked = true;
				break;
				case Process_Wait_Until_Deadline:
				if ((int)(proc->return_deadline - time_read_ticks()) <= 0) unblocked = true;
				break;
				default:
				return 0; // error
				break;
			}
			// if unblocked break
			if (unblocked) {
				proc->status = Process_State_Running;
				break;
			}
		}
		
		// else if process is blocked and it cannot be unblocked
		// advance the queue pointer
		++queue_pointer;
		queue_pointer %= RTOS_MAX_PROCESS_COUNT;
	}
	
	//// all processes are blocked, error has occured
	//if (blocked_count == process_count) return 0;	
	
	// run next unblocked process (if there are unblocked processes
	if (blocked_count != process_count) switch_process(proc);
	
	// move blocked processes up queue
	for (; blocked_count > 0; --blocked_count) {
		int source_pointer = (process_queue_head + blocked_count - 1) % RTOS_MAX_PROCESS_COUNT;
		int destination_pointer = (process_queue_head + blocked_count) % RTOS_MAX_PROCESS_COUNT;
		process_queue[destination_pointer] = process_queue[source_pointer];
	}
	
	--process_count;
	++process_queue_head;
	process_queue_head %= RTOS_MAX_PROCESS_COUNT;
	return proc;
}


void yield_process(int process_status) {
	// set status of process
	current_process->status = process_status;
	
	// save stack pointer of process
	current_process->stack_pointer = stack_pointer;
	
	// set stack pointer to be above the first address in the stack (link register)
	stack_pointer = current_process->stack_base - 4;
	
	// read program counter
	// register stack isn't overwritten
	register uint32_t pc;
	// asm macro
	READ_PROGRAM_COUNTER(pc);
	// save program counter for return (accounting for pipeline)
	current_process->program_counter = pc + 4;
	
	// branch (return) to process caller
	POP_PROGRAM_COUNTER();
	
	// execution will resume here when control is returned by the OS
	DISCARD_REGISTERS;
	
	// set ended to true so OS will know if process ends
	current_process->status = Process_State_Done;
}


//void wait_until(void* variable, uint32_t value, Process_Wait_Until_Condition condition) {
	//// set return deadline to now (as soon as possible)
	//current_process->return_deadline = time_read_ticks();
	//
	//// indicate the process is now blocked
	//current_process->status = Process_State_Blocked;
	//
	//// create structure to define when process resumes execution
	//volatile Process_Wait_Until_Data data = {
		//.variable = variable,
		//.value = value,
		//.condition = condition
	//};
	//
	//// make internal data pointer point to structure
	//current_process->internal_data = &data;
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
//}

void wait_until(void* variable, uint32_t value, uint32_t mask, Process_Wait_Until_Condition condition) {
	// set return deadline to now (as soon as possible)
	current_process->return_deadline = time_read_ticks();
	
	// create structure to define when process resumes execution
	volatile Process_Wait_Until_Data data = {
		.variable = variable,
		.value = value,
		.mask = mask,
		.condition = condition
	};
	
	// make internal data pointer point to structure
	current_process->internal_data = &data;
	
	yield_process(Process_State_Blocked);
}