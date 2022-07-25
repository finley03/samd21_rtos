#include "dma.h"

bool dma_set_channel(int channel) {
	if (channel >= 0 && channel < 12) {
		DMAC->CHID.reg = (uint8_t)channel;
		return true;
	}
	else return false;
}

void dma_init() {
	// enable bus clocks
	PM->AHBMASK.bit.DMAC_ = 1;
	PM->APBBMASK.bit.DMAC_ = 1;
	
	// tell DMAC where root descriptors are 
	DMAC->BASEADDR.reg = (uint32_t)dma_descriptor;
	DMAC->WRBADDR.reg = (uint32_t)dma_descriptor_writeback;
	
	// enable DMAC
	DMAC->CTRL.reg = DMAC_CTRL_LVLEN0 | DMAC_CTRL_LVLEN1 |
		DMAC_CTRL_LVLEN2 | DMAC_CTRL_LVLEN3 |
		DMAC_CTRL_DMAENABLE;
}

bool dma_create_descriptor(DMA_DESCRIPTOR_Type* descriptor, bool incsource, bool incdest,
	uint32_t beatsize, uint16_t count, void* src, void* dst, void* nextdescriptor) {
		
	// check beatsize is in range
	if (beatsize > 0x2) return false;
	
	// set BTCTRL register	
	descriptor->BTCTRL.reg = DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_STEPSEL_DST | 
		((incsource) ? DMAC_BTCTRL_SRCINC : 0) | ((incdest) ? DMAC_BTCTRL_DSTINC : 0) |
		(beatsize << DMAC_BTCTRL_BEATSIZE_Pos) | DMAC_BTCTRL_BLOCKACT_NOACT |
		DMAC_BTCTRL_EVOSEL_DISABLE | DMAC_BTCTRL_VALID;
	
	// set number of beats in transfer
	descriptor->BTCNT = count;
	
	// set source & destination address
	descriptor->SRCADDR = (incsource) ? (uint32_t)src + count * (beatsize + 1) : (uint32_t)src;
	descriptor->DSTADDR = (incdest) ? (uint32_t)dst + count * (beatsize + 1) : (uint32_t)dst;
	
	// set address of next descriptor
	descriptor->DESCADDR = (uint32_t)nextdescriptor;
	
	return true;
}

bool dma_init_channel(int channel, uint32_t trigact, uint32_t trigsrc, int priority) {
	if (!dma_set_channel(channel)) return false;
	if (priority < 0 || priority >= 4) return false;
	if (trigact > 3 || trigact == 1) return false;
	
	DMAC->CHCTRLB.reg = (trigact << DMAC_CHCTRLB_TRIGACT_Pos) | DMAC_CHCTRLB_TRIGSRC(trigsrc) |
		((uint32_t)priority << DMAC_CHCTRLB_LVL_Pos);
		
	return true;
}

bool dma_enable_channel(int channel) {
	if (!dma_set_channel(channel)) return false;
	
	// clear interrupt
	DMAC->CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
	// enable channel
	DMAC->CHCTRLA.bit.ENABLE = 1;
	return true;
}

bool dma_disable_channel(int channel) {
	if (!dma_set_channel(channel)) return false;
	
	// disable channel
	DMAC->CHCTRLA.bit.ENABLE = 0;
	return true;
}

bool dma_suspend_channel(int channel) {
	if (!dma_set_channel(channel)) return false;
	
	// suspend channel
	DMAC->CHCTRLB.reg |= DMAC_CHCTRLB_CMD_SUSPEND;
	return true;
}

bool dma_resume_channel(int channel) {
	if (!dma_set_channel(channel)) return false;
	
	// resume channel
	DMAC->CHCTRLB.reg |= DMAC_CHCTRLB_CMD_RESUME;
	return true;
}

bool dma_trigger_channel(int channel) {
	if (channel >= 0 && channel < 12) {
		DMAC->SWTRIGCTRL.reg = 1 << channel;
		return true;
	}
	else return false;
}

bool dma_transfer_complete(int channel) {
	if (!dma_set_channel(channel)) return false;
	
	// check if transfer is complete
	if (DMAC->CHINTFLAG.bit.TCMPL) return true;
	else return false;
}