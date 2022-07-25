#include "rtos.h"
#include PORT_DRIVER
#include TIME_DRIVER
//#include "usb.h"
#include "dma.h"
#include "uart.h"

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

int main() {
	port_set_output(0, LED_0 | LED_1 | LED_2 | LED_3 | LED_4 | LED_5);
	
	led_on();
	rtos_delay_s(1);
	led_off();
	rtos_delay_s(1);
	
	port_wrconfig(0, PORT_PMUX_C, PORT_PA14 | PORT_PA15);
	if (!uart_init(SERCOM2, 2, 3, 9600)) SOS();
	uart_flush(SERCOM2);
	
	uint8_t value = 0xff;
	uint8_t dst = 0;
	
	dma_init();
	dma_create_descriptor(&(dma_descriptor[0]), false, false, DMA_BEATSIZE_BYTE, 16,
		&(SERCOM2->USART.DATA.reg), &(SERCOM2->USART.DATA.reg), DMA_NEXTDESCRIPTOR_NONE);
	dma_init_channel(0, DMA_TRIGACT_BEAT, SERCOM2_DMAC_ID_RX, 0);
	dma_enable_channel(0);
	
	wait_until(&(DMAC->CHINTFLAG.reg), DMAC_CHINTFLAG_TCMPL, DMAC_CHINTFLAG_TCMPL, Process_Wait_Until_Equal);
	
	for (int i = 0; i < 5; ++i) {
		led_on();
		rtos_delay_ms(200);
		led_off();
		rtos_delay_ms(200);
	}
	
	return 0;
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