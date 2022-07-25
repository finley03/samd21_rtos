#include "rtos.h"
#include PORT_DRIVER
#include TIME_DRIVER
//#include "usb.h"
#include "dma.h"
#include "rtos_dma.h"
#include "uart.h"
#include "spi.h"
#include "eeprom_cat25.h"

#define LED_0 PORT_PA06
#define LED_1 PORT_PA07
#define LED_2 PORT_PA08
#define LED_3 PORT_PA09
#define LED_4 PORT_PA10
#define LED_5 PORT_PA11

bool interrupted;
extern void SOS();
//DMA_DESCRIPTOR_Type desc2 __attribute__((aligned(64)));

//void usb_process();
void blink_led0();

const int test = 2;

int main() {
	port_set_output(PORT_PORTA, LED_0 | LED_1 | LED_2 | LED_3 | LED_4 | LED_5);
	
	Process _blink_led;
	Process* blink_led = &_blink_led;
	init_process(blink_led, blink_led0, 0x600, 0x200);
	dispatch_process(blink_led);
	
	led_on();
	rtos_delay_s(1);
	led_off();
	rtos_delay_s(1);
	
	port_wrconfig(PORT_PORTA, PORT_PMUX_C, PORT_PA14 | PORT_PA15);
	if (!uart_init(SERCOM2, 2, 3, 9600)) SOS();
	uart_flush(SERCOM2);
	
	dma_init();
	dma_create_descriptor(&(dma_descriptor[0]), false, false, DMA_BEATSIZE_BYTE, 16,
		&(SERCOM2->USART.DATA.reg), &(SERCOM2->USART.DATA.reg), DMA_NEXTDESCRIPTOR_NONE);
	dma_init_channel(0, DMA_TRIGACT_BEAT, SERCOM2_DMAC_ID_RX, 0);
	dma_enable_channel(0);
	
	eeprom_init();
	port_wrconfig(PORT_PORTA, PORT_PMUX_C, PORT_PA16 | PORT_PA17 | PORT_PA18);
	if (!spi_init(SERCOM1, 0, 0, 2, 0, 100000)) SOS();
	
	//eeprom_write_byte(1, 0x25);
	//uint8_t val = eeprom_read_byte(1);
	//if (val == 0x25) led_on();
	//else SOS();
	
	rtos_dma_wait_until_end(0);
	
	led_on();
	
	join_process(blink_led);
	
	led_off();
	rtos_delay_s(1);
	
	return 0;
}

void blink_led0() {
	for (int i = 0; i < 100; ++i) {
		port_toggle(PORT_PORTA, LED_0);
		rtos_delay_ms(250);
	}
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