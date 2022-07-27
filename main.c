#include "rtos.h"
#include PORT_DRIVER
#include TIME_DRIVER
//#include "usb.h"
#include "dma.h"
#include "rtos_dma.h"
#include "uart.h"
#include "spi.h"
#include "eeprom_cat25.h"
#include "rtos_dma_eeprom_cat25.h"
#include "util.h"
#include "rtos_dma_spi.h"
#include "com_process.h"

#define LED_0 PORT_PA06
#define LED_1 PORT_PA07
#define LED_2 PORT_PA08
#define LED_3 PORT_PA09
#define LED_4 PORT_PA10
#define LED_5 PORT_PA11

COM_Process spiproc;

bool interrupted;
extern void SOS();
//DMA_DESCRIPTOR_Type desc2 __attribute__((aligned(64)));

//void usb_process();
void blink_led0();
void spi_process_exec_function(COM_Process_Transaction_Request* request, void* port_descriptor);

int test = 2;

int main() {
	port_set_output(PORT_PORTA, LED_0 | LED_1 | LED_2 | LED_3 | LED_4 | LED_5);
	
	led_on();
	rtos_delay_s(2);
	led_off();
	
	port_wrconfig(PORT_PORTA, PORT_PMUX_C, PORT_PA14 | PORT_PA15);
	if (!uart_init(SERCOM2, 2, 3, 9600)) SOS();
	uart_flush(SERCOM2);
	
	EEPROM_CAT25_DESCRIPTOR eeprom_desc = {
		.sercom = SPI_SERCOM,
		.sspin = {
			.pin = PORT_PA19,
			.port = PORT_PORTA
		},
		.writecycletime = 5,
		.size = 0x2000
	};
	DMA_SPI_DESCRIPTOR spi_desc = {
		.txchannel = SPI_DMA_TX_CHANNEL,
		.rxchannel = SPI_DMA_RX_CHANNEL,
		.txtrig = SPI_DMA_TX_TRIGGER,
		.rxtrig = SPI_DMA_RX_TRIGGER,
		.priority = 1
	};
	eeprom_cat25_init(&eeprom_desc);
	port_wrconfig(PORT_PORTA, PORT_PMUX_C, PORT_PA16 | PORT_PA17 | PORT_PA18);
	if (!spi_init(SPI_SERCOM, 0, 0, 2, 0, 100000)) SOS();
	
	dma_init();
	uint8_t values[3] = { 4, 5, 6 };
	//rtos_dma_eeprom_write(12, 2, values);
	//rtos_delay_ms(100);
	//uint8_t wb[8] = {1, 1, 1, 1, 1, 1, 1, 1};
	//rtos_dma_eeprom_read(&eeprom_desc, &spi_desc, 8, 8, wb);
	
	// start com process
	//Process spiproc;
	//COM_PROCESS_DATA spiprocdata;
	new_com_process(&spiproc, 0x600, 0x200, SPI_SERCOM, &spi_desc, spi_process_exec_function);
	
	uint8_t wb[16];
	
	{
		COM_Process_Transaction_Request write_request;
		EEPROM_CAT25_Request eeprom_write_request = {
			.type = EEPROM_CAT25_Write,
			.address = 0x3F,
			.count = 3,
			.data = values
		};
		request_transaction(&spiproc, &write_request, SPI_DEVICE_EEPROM_CAT25, &eeprom_desc, &eeprom_write_request);
		COM_Process_Transaction_Request read_request;
		EEPROM_CAT25_Request eeprom_read_request = {
			.type = EEPROM_CAT25_Read,
			.address = 0x3E,
			.count = 16,
			.data = wb
		};
		request_transaction(&spiproc, &read_request, SPI_DEVICE_EEPROM_CAT25, &eeprom_desc, &eeprom_read_request);
		wait_until(&(read_request.status), COM_Request_Complete, U8_MASK, Process_Wait_Until_Equal);
	}
	// push requests
	//for (int i = 0; i < 8; ++i) {
		//rtos_delay_ms(200);
		//port_set(PORT_PORTA, LED_0);
		//request_transaction(&spiprocdata, &request, 0, &eeprom_desc, &test);
		//port_clear(PORT_PORTA, LED_0);
	//}
	
	
	rtos_delay_s(5);
	led_on();
	
	while(1) {
		
	}
	
	return 0;
}

void spi_process_exec_function(COM_Process_Transaction_Request* request, void* port_descriptor) {
	DMA_SPI_DESCRIPTOR* spi_desc = (DMA_SPI_DESCRIPTOR*)port_descriptor;
	
	bool markcomplete = true;
	
	switch (request->device_id) {
		case SPI_DEVICE_EEPROM_CAT25:
		{
			EEPROM_CAT25_DESCRIPTOR* eeprom_desc = (EEPROM_CAT25_DESCRIPTOR*)(request->device_descriptor);
			EEPROM_CAT25_Request* eeprom_request = (EEPROM_CAT25_Request*)(request->request_data);
			markcomplete = rtos_dma_eeprom_process_request(eeprom_desc, spi_desc, eeprom_request);
			//markcomplete = true;
			if (eeprom_request->type == EEPROM_CAT25_Write) {
				request->busy_until = time_read_ticks() + (uint32_t)(eeprom_desc->writecycletime) * TIME_TICKS_MS_MULT;
				request->busy = true;
			}
		}
		break;
		
		default:
		break;
	}
	
	if (markcomplete) request->status = COM_Request_Complete;
}

//void usb_process() {
	//rtos_delay_s(2);
	//
	//port_set(0, LED_1);
	//
	//interrupted = false;
	//
	//usb_attach();
	//
	//wait_until(&interrupted, true, BOOL_MASK, Process_Wait_Until_Equal);
	//
	//port_clear(0, LED_1);
	//
	//usb_detatch();
//}