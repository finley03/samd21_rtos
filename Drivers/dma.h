#ifndef DMA_H
#define DMA_H

#define DMA_CHANNELS 10

#ifndef DMA_CHANNELS
#error Macro "DMA_CHANNELS" must be defined to a value between 1 and 12
#endif

#include <stdbool.h>
#include <sam.h>

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
} DMA_Descriptor_Type;

typedef struct __attribute__((packed)) {
	uint8_t txchannel;
	uint8_t rxchannel;
	uint8_t rxtrig;
	uint8_t txtrig;
	uint8_t priority;
} DMA_Descriptor;

extern DMA_Descriptor_Type dma_descriptor[DMA_CHANNELS] __attribute__ ((aligned (8)));
extern DMA_Descriptor_Type dma_descriptor_writeback[DMA_CHANNELS] __attribute__ ((aligned (8)));

bool dma_set_channel(uint8_t channel);
void dma_init();

#define DMA_BEATSIZE_BYTE DMAC_BTCTRL_BEATSIZE_BYTE_Val
#define DMA_BEATSIZE_HWORD DMAC_BTCTRL_BEATSIZE_HWORD_Val
#define DMA_BEATSIZE_WORD DMAC_BTCTRL_BEATSIZE_WORD_Val
#define DMA_NEXTDESCRIPTOR_NONE 0

bool dma_create_descriptor(DMA_Descriptor_Type* descriptor, bool incsource, bool incdest,
	uint8_t beatsize, uint16_t count, void* src, void* dst, void* nextdescriptor);
	
#define DMA_TRIGACT_BLOCK DMAC_CHCTRLB_TRIGACT_BLOCK_Val
#define DMA_TRIGACT_BEAT DMAC_CHCTRLB_TRIGACT_BEAT_Val
#define DMA_TRIGACT_TRANSACTION DMAC_CHCTRLB_TRIGACT_TRANSACTION_Val
	
bool dma_init_channel(uint8_t channel, uint8_t trigact, uint8_t trigsrc, uint8_t priority);
bool dma_enable_channel(uint8_t channel);
bool dma_disable_channel(uint8_t channel);
bool dma_suspend_channel(uint8_t channel);
bool dma_resume_channel(uint8_t channel);
bool dma_trigger_channel(uint8_t channel);
bool dma_transfer_complete(uint8_t channel);

#endif