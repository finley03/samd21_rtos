#include "usb_serial.h"
#include "usb.h"


__attribute__((aligned(4))) volatile uint8_t usb_serial_buffer_out[SERIAL_BUFFER_SIZE];
__attribute__((aligned(4))) volatile uint8_t usb_serial_buffer_in[SERIAL_BUFFER_SIZE];

//volatile uint8_t* out_buffers[2] = { usb_serial_buffer_out_a, usb_serial_buffer_out_b };
//volatile uint8_t* in_buffers[2] = { usb_serial_buffer_in_a, usb_serial_buffer_in_b };
	
volatile bool tx_busy;
	
	
void usb_serial_init() {
	// init endpoints
	usb_enable_ep(USB_EP_CDC_NOTIFICATION, USB_EP_TYPE_INTERRUPT, 8);
	usb_enable_ep(USB_EP_CDC_OUT, USB_EP_TYPE_BULK, 64);
	usb_enable_ep(USB_EP_CDC_IN, USB_EP_TYPE_BULK, 64);
	
	// start OUT transfer (host to device)
	usb_ep_start_out(USB_EP_CDC_OUT, usb_serial_buffer_out, SERIAL_BUFFER_SIZE);
	
	tx_busy = false;
}

void usb_serial_out_completion() {
	volatile size_t length = usb_ep_out_length(USB_EP_CDC_OUT);
	usb_ep_start_out(USB_EP_CDC_OUT, usb_serial_buffer_out, 64);
}

void usb_serial_in_completion() {
	tx_busy = false;
}

void usb_serial_send(uint8_t* data, int n) {
	if (n > 64) return;
	
	for (int i = 0; i < n; ++i) {
		usb_serial_buffer_in[i] = data[i];
	}
	
	usb_ep_start_in(USB_EP_CDC_IN, usb_serial_buffer_in, n, false);
	tx_busy = true;
}