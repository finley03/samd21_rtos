#include "rtos_dma_eeprom_cat25.h"
#include "spi.h"
#include "rtos.h"
#include "rtos_dma.h"

#define EEPROM_WRITE_CYCLE_TIME 5

typedef struct __attribute__((packed, scalar_storage_order("big-endian"))) {
	uint8_t command;
	uint16_t address;
} RTOS_DMA_EEPROM_ADDRESSED_REQUEST;

const uint8_t zero = 0;
uint8_t datasink;

void rtos_dma_eeprom_read(EEPROM_CAT25_DESCRIPTOR* eepromdesc, DMA_SPI_DESCRIPTOR* dmadesc, uint32_t address, int count, void* writeback) {
	// create request data
	RTOS_DMA_EEPROM_ADDRESSED_REQUEST request = {
		.command = EEPROM_READ,
		.address = (uint16_t)address
	};
	register Sercom* sercom = eepromdesc->sercom;
	// create descriptor for tx channel
	dma_create_descriptor(&(dma_descriptor[dmadesc->txchannel]), true, false, DMA_BEATSIZE_BYTE, sizeof(request),
		&request, &(sercom->SPI.DATA.reg), &txdesc2);
	dma_create_descriptor(&txdesc2, false, false, DMA_BEATSIZE_BYTE, count,
		&zero, &(sercom->SPI.DATA.reg), DMA_NEXTDESCRIPTOR_NONE);
	// create descriptor for rx channel
	dma_create_descriptor(&(dma_descriptor[dmadesc->rxchannel]), false, false, DMA_BEATSIZE_BYTE, sizeof(request),
		&(sercom->SPI.DATA.reg), &datasink, &rxdesc2);
	dma_create_descriptor(&rxdesc2, false, true, DMA_BEATSIZE_BYTE, count,
		&(sercom->SPI.DATA.reg), writeback, DMA_NEXTDESCRIPTOR_NONE);
		
	rtos_dma_spi_transaction(sercom, &(eepromdesc->sspin), dmadesc);
}

void rtos_dma_eeprom_write(EEPROM_CAT25_DESCRIPTOR* eepromdesc, DMA_SPI_DESCRIPTOR* dmadesc, uint32_t address, int count, void* data) {
	// create request data
	RTOS_DMA_EEPROM_ADDRESSED_REQUEST request = {
		.command = EEPROM_WRITE,
		.address = (uint16_t)address
	};
	register Sercom* sercom = eepromdesc->sercom;
	// create descriptor for tx channel
	dma_create_descriptor(&(dma_descriptor[dmadesc->txchannel]), true, false, DMA_BEATSIZE_BYTE, sizeof(request),
		&request, &(sercom->SPI.DATA.reg), &txdesc2);
	dma_create_descriptor(&txdesc2, true, false, DMA_BEATSIZE_BYTE, count,
		data, &(sercom->SPI.DATA.reg), DMA_NEXTDESCRIPTOR_NONE);
	// create descriptor for rx channel
	dma_create_descriptor(&(dma_descriptor[dmadesc->rxchannel]), false, false, DMA_BEATSIZE_BYTE, sizeof(request),
		&(sercom->SPI.DATA.reg), &datasink, &rxdesc2);
	dma_create_descriptor(&rxdesc2, false, false, DMA_BEATSIZE_BYTE, count,
		&(sercom->SPI.DATA.reg), &datasink, DMA_NEXTDESCRIPTOR_NONE);
	
	eeprom_cat25_write_enable(eepromdesc);
	rtos_dma_spi_transaction(sercom, &(eepromdesc->sspin), dmadesc);
	//rtos_delay_ms(eepromdesc->writecycletime);
	//eeprom_cat25_write_disable(eepromdesc);
}

bool rtos_dma_eeprom_process_request(EEPROM_CAT25_DESCRIPTOR* eepromdesc, DMA_SPI_DESCRIPTOR* dmadesc, EEPROM_CAT25_Request* request) {
	switch (request->type) {
		case EEPROM_CAT25_Read:
		{	
			rtos_dma_eeprom_read(eepromdesc, dmadesc, request->address, request->count, request->data);
		}
		break;
		case EEPROM_CAT25_Write:
		{
			uint32_t top = request->address + request->count;
			uint32_t next_page = (request->address / EEPROM_PAGE_SIZE) * EEPROM_PAGE_SIZE + EEPROM_PAGE_SIZE;
			int count = (top < next_page) ? request->count : next_page - request->address;
			rtos_dma_eeprom_write(eepromdesc, dmadesc, request->address, count, request->data);
			// prepare request to run again
			request->count -= count;
			if (request->count) {
				request->address += count;
				request->data = (uint8_t*)(request->data) + count;
				return false;
			}
		}
		break;
	}
	return true;
}