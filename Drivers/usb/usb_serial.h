#ifndef USB_SERIAL
#define USB_SERIAL

#include <stdint.h>

#define SERIAL_BUFFER_SIZE 8

void usb_serial_init();
void usb_serial_out_completion();
void usb_serial_in_completion();
void usb_serial_send(uint8_t* data, int n);

#endif