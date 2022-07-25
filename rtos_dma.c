#include "rtos_dma.h"
#include "dma.h"
#include "rtos.h"

void rtos_dma_wait_until_end_callback() {
	DMAC->CHID.reg = *((uint8_t*)(current_process->data));
}

void rtos_dma_wait_until_end(int channel) {
	current_process->data = &channel;
	wait_until_callback(&(DMAC->CHINTFLAG.reg), DMAC_CHINTFLAG_TCMPL, DMAC_CHINTFLAG_TCMPL,
		Process_Wait_Until_Equal, rtos_dma_wait_until_end_callback);
}