#ifndef DMA_H
#define DMA_H

#define DMA_CHANNELS 2

#ifndef DMA_CHANNELS
#error Macro "DMA_CHANNELS" must be defined to a value between 1 and 12
#endif

#include <stdbool.h>
#include "samd21.h"

typedef struct __attribute__((aligned(16))) {
	union {
		struct {
			uint16_t VALID:1;
			uint16_t EVOSEL:2;
			uint16_t BLOCKACT:2;
			uint16_t :3;
			uint16_t BEATSIZE:2;
			uint16_t SRCINC:1;
			uint16_t DSTINC:1;
			uint16_t STEPSEL:1;
			uint16_t STEPSIZE:3;
		} bit;
		
		uint16_t reg;
	} BTCTRL;
	
	uint16_t BTCNT;
	uint32_t SRCADDR;
	uint32_t DSTADDR;
	uint32_t DESCADDR;
} DMA_DESCRIPTOR_Type;

volatile DMA_DESCRIPTOR_Type dma_descriptor[DMA_CHANNELS] __attribute__ ((aligned (8)));
DMA_DESCRIPTOR_Type dma_descriptor_writeback[DMA_CHANNELS] __attribute__ ((aligned (8)));

void dma_init();

#define DMA_BEATSIZE_BYTE DMAC_BTCTRL_BEATSIZE_BYTE_Val
#define DMA_BEATSIZE_HWORD DMAC_BTCTRL_BEATSIZE_HWORD_Val
#define DMA_BEATSIZE_WORD DMAC_BTCTRL_BEATSIZE_WORD_Val
#define DMA_NEXTDESCRIPTOR_NONE 0

bool dma_create_descriptor(DMA_DESCRIPTOR_Type* descriptor, bool incsource, bool incdest,
	uint32_t beatsize, uint16_t count, void* src, void* dst, void* nextdescriptor);
	
#define DMA_TRIGACT_BLOCK DMAC_CHCTRLB_TRIGACT_BLOCK_Val
#define DMA_TRIGACT_BEAT DMAC_CHCTRLB_TRIGACT_BEAT_Val
#define DMA_TRIGACT_TRANSACTION DMAC_CHCTRLB_TRIGACT_TRANSACTION_Val
	
bool dma_init_channel(int channel, uint32_t trigact, uint32_t trigsrc, int priority);
bool dma_enable_channel(int channel);
bool dma_disable_channel(int channel);
bool dma_suspend_channel(int channel);
bool dma_resume_channel(int channel);
bool dma_trigger_channel(int channel);
bool dma_transfer_complete(int channel);

#endif