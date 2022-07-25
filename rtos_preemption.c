#include "rtos_preemption.h"
#include "rtos_util.h"
#include "rtos_process.h"


extern Process* current_process;

extern void yield_process(int process_status);

//volatile uint32_t return_address;
////volatile uint32_t link_register;
//volatile uint32_t lr;

//void rtos_preempt_yield() {
	////__asm("mov lr, %0" :: "r" (return_address + 1));
	////register uint32_t apsr;
	////__asm("mrs ")
	////
	////yield_process(Process_State_Running);
	//
	////__asm("mov ")
	//current_process->link_register = link_register;
	//
	//led_toggle();
	//
	//current_process->return_deadline = time_read_ticks();
	//
	//yield_process(Process_State_Running);
	//
	//register uint32_t* storedretaddr = (uint32_t*)(stack_pointer + 0x04);
	//*storedretaddr = current_process->return_address;
	//
	//link_register = current_process->link_register;
	//
	//DISCARD_REGISTERS;
//}

void rtos_preemption_handler() {
	//register uint32_t* storedretaddr = (uint32_t*)(stack_pointer + 0x20);
	////return_address = *((uint32_t*)(stack_pointer + 0x20));
	//current_process->return_address = *storedretaddr;
	//*storedretaddr = (uint32_t)rtos_preempt_yield;
	
	preempt_clear_interrupts();
	
	current_process->return_deadline = time_read_ticks();
	
	yield_process(Process_State_Running);
}


void rtos_enable_preempt() {
	current_process->enable_preempt = true;
	preempt_enable_interrupts();
}

void rtos_disable_preempt() {
	preempt_disable_interrupts();
	current_process->enable_preempt = false;
}