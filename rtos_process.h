#ifndef RTOS_PROCESS_H
#define RTOS_PROCESS_H

#include "rtos_config.h"
#include <stdbool.h>
#include <stdint.h>

#define U32_MASK 0xFFFFFFFF
#define U16_MASK 0x0000FFFF
#define U8_MASK 0x000000FF
#define INT_MASK U32_MASK
#define FLOAT_MASK U32_MASK
#define CHAR_MASK U8_MASK
#define BOOL_MASK U8_MASK
#define SIZE_T_MASK U32_MASK

typedef enum {
	Process_State_Ready,
	Process_State_Running,
	Process_State_Blocked,
	Process_State_Done
} Process_State;

typedef enum {
	Process_Wait_Until_Equal,
	Process_Wait_Until_NotEqual,
	Process_Wait_Until_Greater,
	Process_Wait_Until_GEqual,
	Process_Wait_Until_Less,
	Process_Wait_Until_LEqual,
	//Process_Wait_Until_Deadline
	Process_Wait_Until_None
} Process_Wait_Until_Condition;

typedef struct {
	// variable to check for condition
	uint32_t* variable;
	
	// value to check the variable against
	uint32_t value;
	
	// mask placed over variable for comparison
	// if mask is zero variable is not checked
	uint32_t mask;
	
	// callback that is executed before checking the variable
	void (*callback)(void);
	
	// condition to check
	Process_Wait_Until_Condition condition;
} Process_Wait_Until_Data;

typedef struct {
	// current program counter of the process
	// only valid when the process is not running
	uint32_t program_counter;
	
	// base of the stack space allocated to the process
	uint32_t stack_base;
	
	// size of the stack space allocated to the process
	uint32_t stack_size;
	
	// current stack pointer of the process
	// only valid when the process is not running
	uint32_t stack_pointer;
	
	// function pointer to the process function
	void (*procFunction)(void);
	
	// in cooperative multitasking, this indicates when
	// the process needs to continue execution by
	uint32_t return_deadline;
	
	//// in preemptive multitasking this stores the address
	//// to return to
	//uint32_t return_address;
	//
	//// in preemptive multitasking this stores the link
	//// register of the program
	//uint32_t link_register;
	
	// current status of the process
	// valid whenever the OS has control, or for
	// other processes. invalid for the current
	// process when it is running.
	// should be Process_State type but the compiler
	// optimizes it too much so its not an int
	// just treat it as the enum type
	int status;
	
	// pointer to process specific data used by the
	// operating system
	// should not be used or modified by processes
	void* internal_data;
	
	// pointer to process specific data for the
	// procFunction to access
	void* data;
	
	#ifdef RTOS_PREEMPT
	// true if preemption is enabled for the process
	bool enable_preempt;
	#endif
} Process;

//----------init_process : function----------//
// Function: to initialize the process given and set its function,
// stack position, and stack size.
// Thread safety: completely thread safe
void init_process(Process* process, void (*procFunction)(void), uint32_t stack_base, uint32_t stack_size);

//----------reset_process : function----------//
// Function: to reset a process which has already run so it is
// ready to run again
// Thread safety: completely thread safe
void reset_process(Process* process);

//----------dispatch_process : function----------//
// Function: to send a process to the process queue to be executed
// Return value: true if the process is dispatched to the process queue.
// false if the process is discarded for any reason.
// Thread safety: completely thread safe
bool dispatch_process(Process* process);

//----------wait_until : function----------//
// Function: block the current thread until the given condition is met
// Thread safety: thread safe for any process
void wait_until(void* variable, uint32_t value, uint32_t mask, Process_Wait_Until_Condition condition);

//----------wait_until : function----------//
// Function: block the current thread until the given condition is met.
// Before the condition is checked, the callback function is run.
// Thread safety: thread safe for any process
void wait_until_callback(void* variable, uint32_t value, uint32_t mask, Process_Wait_Until_Condition condition, void (*callback)(void));

//----------join_process : macro----------//
// Function: Shorthand for the function wait_until.
// Blocks the current thread until the process given has finished execution.
// Thread safety: thread safe for any process, however make sure you don't try and join
// the current process, as that will permanently stop execution
#define join_process(proc) wait_until(&((proc)->status), Process_State_Done, U32_MASK, Process_Wait_Until_Equal);

//----------wait_until_started : macro----------//
// Function: Shorthand for the function wait_until.
// Blocks the current thread until the process given has begun execution.
// This doesn't mean the state has become Process_State_Running, it simply
// means it is no longer Process_State_Waiting
// Thread safety: completely thread safe for any process. Even passing the current
// process will not block it, although it will have no function.
#define wait_until_started(proc) wait_until(&((proc)->status), Process_State_Ready, U32_MASK, Process_Wait_Until_NotEqual);

//----------wait_until_true : macro----------//
// Function: Shorthand for the function wait_until.
// Blocks the current thread until the given boolean variable is true
// Thread safety: completely thread safe for any process. Even passing the current
// process will not block it, although it will have no function.
#define wait_until_true(pointer) wait_until(pointer, false, BOOL_MASK, Process_Wait_Until_NotEqual);

//----------wait_until_false : macro----------//
// Function: Shorthand for the function wait_until.
// Blocks the current thread until the given boolean variable is false
// Thread safety: completely thread safe for any process. Even passing the current
// process will not block it, although it will have no function.
#define wait_until_false(pointer) wait_until(pointer, false, BOOL_MASK, Process_Wait_Until_Equal);

//----------current_process : variable----------//
// Description: Pointer to the Process structure for the current process.
// Should be used when trying to access the data pointer for the current
// process. The rest should not be modified, and is not guaranteed to be
// accurate while the current thread is executing.
extern Process* current_process;

#endif