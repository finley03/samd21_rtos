#include "usb_serial.h"
#include "usb.h"

__attribute__((aligned(4))) volatile uint8_t usb_serial_buffer_out[USB_SERIAL_BUFFER_SIZE];
__attribute__((aligned(4))) volatile uint8_t usb_serial_buffer_in[USB_SERIAL_BUFFER_SIZE];
	
volatile bool usb_tx_busy;
volatile size_t usb_rx_buffer_length;
	
void usb_serial_init() {
	// init endpoints
	usb_enable_ep(USB_EP_CDC_NOTIFICATION, USB_EP_TYPE_INTERRUPT, 8);
	usb_enable_ep(USB_EP_CDC_OUT, USB_EP_TYPE_BULK, USB_SERIAL_BUFFER_SIZE);
	usb_enable_ep(USB_EP_CDC_IN, USB_EP_TYPE_BULK, USB_SERIAL_BUFFER_SIZE);
	
	// start OUT transfer (host to device)
	usb_ep_start_out(USB_EP_CDC_OUT, usb_serial_buffer_out, USB_SERIAL_BUFFER_SIZE);
	
	usb_tx_busy = false;
}

volatile size_t usb_rx_buffer_length;

void usb_serial_out_completion() {
	usb_rx_buffer_length = usb_ep_out_length(USB_EP_CDC_OUT);
}

void usb_serial_in_completion() {
	usb_tx_busy = false;
}

int usb_serial_send_buffer(uint8_t* data, int n) {
	n = (n > USB_SERIAL_BUFFER_SIZE) ? USB_SERIAL_BUFFER_SIZE : n;
	
	//for (int i = 0; i < n; ++i) {
		//usb_serial_buffer_in[i] = data[i];
	//}
	memcpy(usb_serial_buffer_in, data, n);
	
	usb_ep_start_in(USB_EP_CDC_IN, usb_serial_buffer_in, n, false);
	usb_tx_busy = true;
	
	return n;
}

int usb_serial_read_buffer(uint8_t* data, int n) {
	n = (n > usb_rx_buffer_length) ? usb_rx_buffer_length : n;
	
	memcpy(data, usb_serial_buffer_out, n);
	
	usb_ep_start_out(USB_EP_CDC_OUT, usb_serial_buffer_out, USB_SERIAL_BUFFER_SIZE);
	
	usb_rx_buffer_length = 0;
	return n;
}