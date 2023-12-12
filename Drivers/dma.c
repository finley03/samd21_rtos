#include "dma.h"

DMA_Descriptor_Type dma_descriptor[DMA_CHANNELS] __attribute__ ((aligned (8)));
DMA_Descriptor_Type dma_descriptor_writeback[DMA_CHANNELS] __attribute__ ((aligned (8)));

bool dma_set_channel(uint8_t channel) {
	if (channel < 12) {
		DMAC_REGS->DMAC_CHID = channel;
		return true;
	}
	else return false;
}

void dma_init() {
	// enable bus clocks
	PM_REGS->PM_AHBMASK |= PM_AHBMASK_DMAC(1);
	PM_REGS->PM_APBBMASK |= PM_AHBMASK_DMAC(1);
	
	// tell DMAC where root descriptors are 
	DMAC_REGS->DMAC_BASEADDR = (uint32_t)dma_descriptor;
	DMAC_REGS->DMAC_WRBADDR = (uint32_t)dma_descriptor_writeback;
	
	// enable DMAC
	DMAC_REGS->DMAC_CTRL = DMAC_CTRL_LVLEN0(1) | DMAC_CTRL_LVLEN1(1) |
		DMAC_CTRL_LVLEN2(1) | DMAC_CTRL_LVLEN3(1) |
		DMAC_CTRL_DMAENABLE(1);
}

bool dma_create_descriptor(DMA_Descriptor_Type* descriptor, bool incsource, bool incdest,
	uint8_t beatsize, uint16_t count, void* src, void* dst, void* nextdescriptor) {
		
	// check beatsize is in range
	if (beatsize > 0x2) return false;
	
	// set BTCTRL register	
	descriptor->BTCTRL.reg = DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_STEPSEL_DST | 
		((incsource) ? DMAC_BTCTRL_SRCINC(1) : 0) | ((incdest) ? DMAC_BTCTRL_DSTINC(1) : 0) |
		DMAC_BTCTRL_BEATSIZE((uint32_t)beatsize) | DMAC_BTCTRL_BLOCKACT_NOACT |
		DMAC_BTCTRL_EVOSEL_DISABLE | DMAC_BTCTRL_VALID(1);
	
	// set number of beats in transfer
	descriptor->BTCNT = count;
	
	// set source & destination address
	descriptor->SRCADDR = (incsource) ? (uint32_t)src + count * (beatsize + 1) : (uint32_t)src;
	descriptor->DSTADDR = (incdest) ? (uint32_t)dst + count * (beatsize + 1) : (uint32_t)dst;
	
	// set address of next descriptor
	descriptor->DESCADDR = (uint32_t)nextdescriptor;
	
	return true;
}

bool dma_init_channel(uint8_t channel, uint8_t trigact, uint8_t trigsrc, uint8_t priority) {
	if (!dma_set_channel(channel)) return false;
	if (priority > 3) return false;
	if (trigact > 3 || trigact == 1) return false;
	
	DMAC_REGS->DMAC_CHCTRLB = DMAC_CHCTRLB_TRIGACT((uint32_t)trigact) | DMAC_CHCTRLB_TRIGSRC((uint32_t)trigsrc) |
		((uint32_t)priority << DMAC_CHCTRLB_LVL_Pos);
		
	return true;
}

bool dma_enable_channel(uint8_t channel) {
	if (!dma_set_channel(channel)) return false;
	
	// clear interrupt
	DMAC_REGS->DMAC_CHINTFLAG = DMAC_CHINTFLAG_TCMPL(1);
	// enable channel
	DMAC_REGS->DMAC_CHCTRLA |= DMAC_CHCTRLA_ENABLE(1);
	return true;
}

bool dma_disable_channel(uint8_t channel) {
	if (!dma_set_channel(channel)) return false;
	
	// disable channel
	DMAC_REGS->DMAC_CHCTRLA &= ~DMAC_CHCTRLA_ENABLE(1);
	return true;
}

bool dma_suspend_channel(uint8_t channel) {
	if (!dma_set_channel(channel)) return false;
	
	// suspend channel
	DMAC_REGS->DMAC_CHCTRLB |= DMAC_CHCTRLB_CMD_SUSPEND;
	return true;
}

bool dma_resume_channel(uint8_t channel) {
	if (!dma_set_channel(channel)) return false;
	
	// resume channel
	DMAC_REGS->DMAC_CHCTRLB |= DMAC_CHCTRLB_CMD_RESUME;
	return true;
}

bool dma_trigger_channel(uint8_t channel) {
	if (channel < 12) {
		DMAC_REGS->DMAC_SWTRIGCTRL = 1 << channel;
		return true;
	}
	else return false;
}

bool dma_transfer_complete(uint8_t channel) {
	if (!dma_set_channel(channel)) return false;
	
	// check if transfer is complete
	if (DMAC_REGS->DMAC_CHINTFLAG & DMAC_CHINTFLAG_TCMPL(1));
	else return false;
}