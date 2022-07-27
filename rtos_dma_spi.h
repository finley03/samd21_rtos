#ifndef RTOS_DMA_SPI
#define RTOS_DMA_SPI

#include "dma.h"
#include "port.h"

DMA_DESCRIPTOR_Type txdesc2;
DMA_DESCRIPTOR_Type rxdesc2;

typedef struct __attribute__((packed)) {
	uint8_t txchannel;
	uint8_t rxchannel;
	uint8_t rxtrig;
	uint8_t txtrig;
	uint8_t priority;
} DMA_SPI_DESCRIPTOR;

void rtos_dma_spi_transaction(Sercom* sercom, PIN* sspin, DMA_SPI_DESCRIPTOR* desc);

#endif