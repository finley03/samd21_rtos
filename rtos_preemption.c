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

//static volatile uint32_t stored_lr;
//static volatile uint32_t stored_xpsr;
//
//__attribute__((optimize("-Og"))) void rtos_yield_process() {
	//__disable_irq();
		//
	//// set status of process
	//current_process->status = Process_State_Running;
	//
	//current_process->return_deadline = time_read_ticks();
		//
	//// save stack pointer of process
	//current_process->stack_pointer = stack_pointer - 8;
		//
	//// set stack pointer to be above the first address in the stack (link register)
	//stack_pointer = current_process->stack_base - 4;
		//
	//// read program counter
	//// register stack isn't overwritten
	//register uint32_t pc;
	//// asm macro
	//READ_PROGRAM_COUNTER(pc);
	//// save program counter for return (accounting for pipeline)
	//current_process->program_counter = pc + 4; // 4
		//
	//// branch (return) to process caller
	//POP_PROGRAM_COUNTER();
		//
	//// execution will resume here when control is returned by the OS
	//DISCARD_REGISTERS;
		//
	//stack_pointer += 8;
		//
	//// set ended to true so OS will know if process ends
	//current_process->status = Process_State_Done;
	//
	//link_register = stored_lr;
	////__asm("MSR APSR_nzcvq, %0" :: "r" (stored_xpsr));
	//__asm("MSR PSR, %0" :: "r" (stored_xpsr));
		//
	//__enable_irq();
//}
//
//void rtos_preemption_handler() {
	////register uint32_t* storedretaddr = (uint32_t*)(stack_pointer + 0x20);
	//////return_address = *((uint32_t*)(stack_pointer + 0x20));
	////current_process->return_address = *storedretaddr;
	////*storedretaddr = (uint32_t)rtos_preempt_yield;
	//
	//__disable_irq();
	//preempt_clear_interrupts();
	//
	//
	//uint32_t sp = __get_PSP();
	//uint32_t interrupt_stack = sp;
	//stored_lr = *(uint32_t*)(interrupt_stack + 0x14);
	//*(uint32_t*)(interrupt_stack + 0x14) = *(uint32_t*)(interrupt_stack + 0x18);
	//*(uint32_t*)(interrupt_stack + 0x18) = (uint32_t)rtos_yield_process;
	//stored_xpsr = *(uint32_t*)(interrupt_stack + 0x1C);
//}

//__attribute__((optimize("-Og"))) void preempt_yield_process(int process_status) {
	//__disable_irq();
	//
	//// set status of process
	//current_process->status = process_status;
	//
	//// set stack pointer to be above the first address in the stack (link register)
	//stack_pointer = current_process->stack_base - 4;
	//
	//// read program counter
	//// register stack isn't overwritten
	//register uint32_t pc;
	//// asm macro
	//READ_PROGRAM_COUNTER(pc);
	//// save program counter for return (accounting for pipeline)
	//current_process->program_counter = pc + 4; // 4
	//
	//// branch (return) to process caller
	//POP_PROGRAM_COUNTER();
	//
	//// execution will resume here when control is returned by the OS
	//DISCARD_REGISTERS;
	//
	//stack_pointer += 8;
	//
	//// set ended to true so OS will know if process ends
	//current_process->status = Process_State_Done;
	//
	//__enable_irq();
//}

//__attribute__((naked)) void rtos_preemption_handler() {
	//__asm(
		//"cpsid i\n" // disable interrupts
		//"mrs r0, psp\n" // move psp to r0
		//"sub r0, #16\n" // subtract 16 (4 registers) from r0
		//"stmia r0!, {r4-r7}\n" // store registers r4 to r7 while incrementing addresses
		//"mov r4, r8\n" // shift registers down
		//"mov r5, r9\n"
		//"mov r6, r10\n"
		//"mov r7, r11\n"
		//"sub r0, #32\n"
		//"stmia r0!, {r4-r7}\n" // store rest of the registers
		//"sub r0, #16\n" // 16 plus
	//);
	//
	//
	//
	////// save psp of previous process
	////__asm("mov %0, r0" : "=r" (current_process->stack_pointer));
	////
	////if (current_process->status == Process_State_Done) current_process->status = Process_State_Running;
	////
	////// set stack pointer to be above the first address in the stack (link register)
	////stack_pointer = current_process->stack_base - 4;
	////
	////// read program counter
	////// register stack isn't overwritten
	////register uint32_t pc;
	////// asm macro
	////READ_PROGRAM_COUNTER(pc);
	////// save program counter for return (accounting for pipeline)
	//////current_process->program_counter = pc + 4; // 4
	////process_pc = pc + 6;
	////
	////// branch (return) to process caller
	////POP_PROGRAM_COUNTER();
	////
	////// execution will resume here when control is returned by the OS
	////DISCARD_REGISTERS;
	////
	////// set ended to true so OS will know if process ends
	////current_process->status = Process_State_Done;
	//
	//
	//// load psp of new process
	//__asm("mov r0, %0" :: "r" (current_process->stack_pointer));
	//
	//// restore registers
	//__asm(
		//"ldmia r0!, {r4-r7}\n" // load registers
		//"mov r8, r4\n" // shift registers
		//"mov r9, r5\n"
		//"mov r10, r6\n"
		//"mov r11, r7\n"
		//"ldmia r0!, {r4-r7}\n" // load registers
		//"msr psp, r0" // move r0 to psp
	//);
	//
	//__asm(
		//"ldr r0, =0xFFFFFFFD\n"
		//"cpsie i\n" // enable interrupts
		//"bx r0\n"
	//);
//}

void rtos_preemption_handler() {
	preempt_clear_interrupts();
	
	current_process->return_deadline = time_read_ticks();
	
	yield_process(Process_State_Running);
}

void enable_preempt() {
	current_process->enable_preempt = true;
	preempt_enable_interrupts();
}

void disable_preempt() {
	current_process->enable_preempt = false;
	preempt_disable_interrupts();
}