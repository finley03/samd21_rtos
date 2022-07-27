#include "rtos_dma_spi.h"
#include "rtos_dma.h"
#include "spi.h"
#include "eeprom_cat25.h"
#include "rtos_dma_eeprom_cat25.h"

DMA_DESCRIPTOR_Type txdesc2;
DMA_DESCRIPTOR_Type rxdesc2;

//void rtos_dma_spi_transfer_prep() {
	//// set dma channel
	//dma_set_channel(EEPROM_DMA_TX_CHANNEL);
	//// disable if enabled
	//if (DMAC->CHCTRLA.bit.ENABLE) dma_disable_channel(EEPROM_DMA_TX_CHANNEL);
	//// set dma channel
	//dma_set_channel(EEPROM_DMA_RX_CHANNEL);
	//// disable if enabled
	//if (DMAC->CHCTRLA.bit.ENABLE) dma_disable_channel(EEPROM_DMA_RX_CHANNEL);
	//// flush spi
	//spi_flush(EEPROM_SERCOM);
//}
//
//void rtos_dma_spi_transaction() {
	//rtos_dma_spi_transfer_prep();
	//// init for correct parameters
	//dma_init_channel(EEPROM_DMA_TX_CHANNEL, DMA_TRIGACT_BEAT, EEPROM_SERCOM_TX_TRIGGER, EEPROM_DMA_PRIORITY);
	//dma_init_channel(EEPROM_DMA_RX_CHANNEL, DMA_TRIGACT_BEAT, EEPROM_SERCOM_RX_TRIGGER, EEPROM_DMA_PRIORITY);
	//// set ss low
	//port_clear(EEPROM_SS_PORT, EEPROM_SS_PIN);
	//// start transfer
	//dma_enable_channel(EEPROM_DMA_TX_CHANNEL);
	//dma_enable_channel(EEPROM_DMA_RX_CHANNEL);
	//// wait until transfer done
	//rtos_dma_wait_until_end(EEPROM_DMA_RX_CHANNEL);
	//// disable channels
	//dma_disable_channel(EEPROM_DMA_TX_CHANNEL);
	//dma_disable_channel(EEPROM_DMA_RX_CHANNEL);
	//// set ss high
	//port_set(EEPROM_SS_PORT, EEPROM_SS_PIN);
//}

void rtos_dma_spi_transaction(Sercom* sercom, PIN* sspin, DMA_SPI_DESCRIPTOR* desc) {
	// disable channels
	dma_disable_channel(desc->txchannel);
	dma_disable_channel(desc->rxchannel);
	// flush spi
	spi_flush(sercom);
	// init for correct parameters
	dma_init_channel(desc->txchannel, DMA_TRIGACT_BEAT, desc->txtrig, desc->priority);
	dma_init_channel(desc->rxchannel, DMA_TRIGACT_BEAT, desc->rxtrig, desc->priority);
	// set ss low
	port_clear(sspin->port, sspin->pin);
	// start transfer
	dma_enable_channel(desc->txchannel);
	dma_enable_channel(desc->rxchannel);
	// wait until transfer done
	rtos_dma_wait_until_end(desc->rxchannel);
	// disable channels
	dma_disable_channel(desc->txchannel);
	dma_disable_channel(desc->rxchannel);
	// set ss high
	port_set(sspin->port, sspin->pin);
}