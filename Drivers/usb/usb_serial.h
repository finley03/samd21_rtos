#ifndef USB_SERIAL
#define USB_SERIAL

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define USB_SERIAL_BUFFER_SIZE 32

extern volatile bool usb_tx_busy;
extern volatile size_t usb_rx_buffer_length;

void usb_serial_init();
void usb_serial_out_completion();
void usb_serial_in_completion();
int usb_serial_send_buffer(uint8_t* data, int n); // returns number of bytes sent
int usb_serial_read_buffer(uint8_t* data, int n); // returns number of bytes received

#endif