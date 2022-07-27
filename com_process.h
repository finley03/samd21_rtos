#ifndef COM_PROCESS_H
#define COM_PROCESS_H

#include <stdbool.h>
#include "samd21.h"
#include "rtos.h"

#define COM_PROCESS_REQUEST_QUEUE_LENGTH 4

typedef enum {
	COM_Request_Ready,
	COM_Request_Blocked,
	COM_Request_Complete
} COM_Request_Status;

typedef struct __attribute__((packed)) {
	// device identifier
	int device_id;
	// descriptor for device
	void* device_descriptor;
	// data of request
	void* request_data;
	// time device is busy until
	uint32_t busy_until;
	// device busy executing request
	bool busy;
	// request status
	int8_t status;
} COM_Process_Transaction_Request;

typedef struct {
	// sercom process is responsible for
	Sercom* sercom;
	// port descriptor
	void* port_descriptor;
	// execute request function
	void (*execute_function)(COM_Process_Transaction_Request*, void*);
	// request queue
	COM_Process_Transaction_Request* request_queue[COM_PROCESS_REQUEST_QUEUE_LENGTH];
	// queue head
	int request_queue_head;
	// queue tail
	int request_queue_tail;
	// number of requests in queue
	int request_count;
} COM_Process_Data;

typedef struct {
	Process process;
	COM_Process_Data data;
} COM_Process;

// create new com process
// execute_function arguments must be in the form of (request, port_descriptor)
void new_com_process(COM_Process* proc, uint32_t stack_base, uint32_t stack_size, Sercom* sercom,
	void* port_descriptor, void (*execute_function)(COM_Process_Transaction_Request*, void*));
	
void request_transaction(COM_Process* proc, COM_Process_Transaction_Request* request,
	int device_id, void* device_descriptor, void* request_data);

#endif