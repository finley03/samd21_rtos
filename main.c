#include "rtos.h"
#include PORT_DRIVER
#include TIME_DRIVER
#include "dma.h"
#include "rtos_dma.h"
#include "uart.h"
#include "spi.h"
#include "eeprom_cat25.h"
#include "rtos_dma_eeprom_cat25.h"
#include "util.h"
#include "rtos_dma_spi.h"
#include "com_process.h"
#include "usb.h"
#include "usb_serial.h"
#include "i2c.h"
#include "pwm.h"

COM_Process spiproc;

bool interrupted;
extern void SOS();

void blink_led0();
void spi_process_exec_function(COM_Process_Transaction_Request* request, void* port_descriptor);

void usb_proc_loop();
extern void usb_handle_function();

Process usbproc;

void led_blinker();

int main() {
	//port_set_output(PORT_PORTA, LED_0 | LED_1);
	
	//port_wrconfig(PORT_PORTA, PORT_PMUX_C, PORT_PA14 | PORT_PA15);
	//if (!uart_init(SERCOM2, 2, 3, 9600)) SOS();
	//uart_flush(SERCOM2);
	
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
	//
	//port_wrconfig(PORT_PORTA, PORT_PMUX_C, PORT_PA08 | PORT_PA09);
	//i2c_init(SERCOM0);
	
	led_on();
	rtos_delay_s(2);
	led_off();
	rtos_delay_s(1);
	
	dma_init();
	uint8_t values[3] = { 4, 5, 6 };
	
	//new_com_process(&spiproc, 0x600, 0x200, SPI_SERCOM, &spi_desc, spi_process_exec_function);
	
	//uint8_t wb[16];
	//
	//{
		//COM_Process_Transaction_Request write_request;
		//EEPROM_CAT25_Request eeprom_write_request = {
			//.type = EEPROM_CAT25_Write,
			//.address = 0x3F,
			//.count = 3,
			//.data = values
		//};
		//request_transaction(&spiproc, &write_request, SPI_DEVICE_EEPROM_CAT25, &eeprom_desc, &eeprom_write_request);
		//COM_Process_Transaction_Request read_request;
		//EEPROM_CAT25_Request eeprom_read_request = {
			//.type = EEPROM_CAT25_Read,
			//.address = 0x3E,
			//.count = 16,
			//.data = wb
		//};
		//request_transaction(&spiproc, &read_request, SPI_DEVICE_EEPROM_CAT25, &eeprom_desc, &eeprom_read_request);
		//wait_until(&(read_request.status), COM_Request_Complete, U8_MASK, Process_Wait_Until_Equal);
	//}
	//
	//
	//rtos_delay_s(5);
	//led_on();
	//
	//while(1) {
		//
	//}
	
	NVIC_DisableIRQ(USB_IRQn);
	port_wrconfig(PORT_PORTA, PORT_PMUX_G, PORT_PA24 | PORT_PA25);
	usb_init();
	usb_attach();
	
	//usbproc = &_usbproc;
	init_process(&usbproc, usb_proc_loop, 0x800, 0x200);
	dispatch_process(&usbproc);
	NVIC_EnableIRQ(USB_IRQn);
	
	
	port_wrconfig(PORT_PORTA, PORT_PMUX_C, PORT_PA08 | PORT_PA09);
	i2c_init(SERCOM0);
	rtos_delay_ms(100);
	uint8_t command = 0x1E;
	// reset
	i2c_send_data(SERCOM0, 0x77, &command, 1);
	rtos_delay_ms(3);
	// read prom
	command = 0b10100110;
	i2c_send_data(SERCOM0, 0x77, &command, 1);
	rtos_delay_ms(2);
	//delay_ms(2);
	uint8_t data[2];
	i2c_receive_data(SERCOM0, 0x77, data, 2);
	
	
	Process blinker;
	init_process(&blinker, led_blinker, 0xA00, 0x100);
	blinker.enable_preempt = true;
	dispatch_process(&blinker);
	
	
	
	while(1) {
		rtos_delay_ms(125);
		led_toggle();
	}
	
	return 0;
}

bool usb_interrupt;

void usb_proc_loop() {
	usb_interrupt = false;
	
	// contained in loop
	while (1) {
		wait_until(&usb_interrupt, true, BOOL_MASK, Process_Wait_Until_Equal);
		usb_interrupt = false;
		
		usb_handle_function();
	}
}

void USB_Handler() {
	NVIC_DisableIRQ(USB_IRQn);
	
	usb_interrupt = true;
	//reset_process(&usbproc);
	//dispatch_process(&usbproc);
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

void led_blinker() {
	port_set_output(PORT_PORTA, PORT_PA14);
	
	while (1) {
	delay_ms(500);
		port_toggle(PORT_PORTA, PORT_PA14);
	}
}