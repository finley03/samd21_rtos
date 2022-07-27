#ifndef RTOS_DMA_EEPROM_CAT25_H
#define RTOS_DMA_EEPROM_CAT25_H

#include <stdbool.h>
#include "samd21.h"
#include "eeprom_cat25.h"
#include "rtos_dma_spi.h"

//#define EEPROM_SERCOM_TX_TRIGGER SERCOM1_DMAC_ID_TX
//#define EEPROM_SERCOM_RX_TRIGGER SERCOM1_DMAC_ID_RX
//#define EEPROM_DMA_PRIORITY 1
//#define EEPROM_DMA_TX_CHANNEL 0
//#define EEPROM_DMA_RX_CHANNEL 1

typedef enum {
	EEPROM_CAT25_Read,
	EEPROM_CAT25_Write
} EEPROM_CAT25_Transaction_Type;

typedef struct {
	uint32_t address;
	int count;
	void* data;
	EEPROM_CAT25_Transaction_Type type;
} EEPROM_CAT25_Request;

// page safe
void rtos_dma_eeprom_read(EEPROM_CAT25_DESCRIPTOR* eepromdesc, DMA_SPI_DESCRIPTOR* dmadesc, uint32_t address, int count, void* writeback);
// NOT page safe
void rtos_dma_eeprom_write(EEPROM_CAT25_DESCRIPTOR* eepromdesc, DMA_SPI_DESCRIPTOR* dmadesc, uint32_t address, int count, void* data);
// process request, should be called until request is marked as complete, taking into account busy_until
// is page safe
// returns true if request is completed
// false if not
bool rtos_dma_eeprom_process_request(EEPROM_CAT25_DESCRIPTOR* eepromdesc, DMA_SPI_DESCRIPTOR* dmadesc, EEPROM_CAT25_Request* request);

#endif