#include "usb.h"
#include "usb_samd.h"
#include "cdc_standard.h"
#include "usb_serial.h"

// macro that sets endpoint number stuff
USB_ENDPOINTS(3)

#define USB_VID 0x1209
#define USB_PID 0x000E

#define CDC_CONTROL_INTERFACE 0
#define CDC_DATA_INTERFACE 1

#define USB_DATA_OUT_ENDPOINT 1
#define USB_DATA_IN_ENDPOINT 2

#define REQ_PWR 0x10
#define REQ_INFO 0x30
#define REQ_BOOT 0xBB
#define REQ_RESET 0xBD
#define REQ_OPENWRT_BOOT_STATUS 0xBC

// main device descriptor
__attribute__((aligned(4))) const USB_DeviceDescriptor device_descriptor = {
	// length of packet
	.bLength = sizeof(USB_DeviceDescriptor),
	// type of descriptor
	.bDescriptorType = USB_DTYPE_Device,
	
	// usb spec release number (2.0)
	.bcdUSB = 0x0200,
	// misc device
	.bDeviceClass = 0xEF,
	// CDC
	.bDeviceSubClass = 0x02,
	// interface association descriptor
	.bDeviceProtocol = 0x01,
	
	// max packet size for endpoint 0
	.bMaxPacketSize0 = 64,
	// usb VID
	.idVendor = USB_VID,
	// usb PID
	.idProduct = USB_PID,
	// device version number (2.0) GNC2
	.bcdDevice = 0x0200,
	// manufacturer string is string 1
	.iManufacturer = 1,
	// product string is string 2
	.iProduct = 2,
	// serial number string is string 3
	.iSerialNumber = 3,
	
	// only one configuration
	.bNumConfigurations = 1
};

typedef struct {
	USB_ConfigurationDescriptor config;
	
	USB_InterfaceAssociationDescriptor CDC_IAD;
	
	// control interface
	USB_InterfaceDescriptor CDC_control_interface;
	CDC_FunctionalHeaderDescriptor CDC_functional_header;
	CDC_FunctionalACMDescriptor CDC_functional_ACM;
	CDC_FunctionalUnionDescriptor CDC_functional_union;
	USB_EndpointDescriptor CDC_notification_endpoint;
	
	// data interface
	USB_InterfaceDescriptor CDC_data_interface;
	USB_EndpointDescriptor CDC_out_endpoint;
	USB_EndpointDescriptor CDC_in_endpoint;
} USB_FullConfigurationDescriptor;

USB_FullConfigurationDescriptor full_config = {
	.config = {
		// length of config descriptor
		.bLength = sizeof(USB_ConfigurationDescriptor),
		// type of descriptor
		.bDescriptorType = USB_DTYPE_Configuration,
		// length of full_config descriptor
		.wTotalLength = sizeof(USB_FullConfigurationDescriptor),
		// number of interfaces (2) CDC control interface, CDC data interface
		.bNumInterfaces = 2,
		// configuration value (1)
		.bConfigurationValue = 1,
		// no configuration string
		.iConfiguration = 0,
		// self powered
		.bmAttributes = USB_CONFIG_ATTR_SELFPOWERED | 0x80, // bit 7 must be set
		// power
		.bMaxPower = USB_CONFIG_POWER_MA(0)
	},
	
	.CDC_IAD = {
		// length of IAD descriptor
		.bLength = sizeof(USB_InterfaceAssociationDescriptor),
		// type of descriptor
		.bDescriptorType = USB_DTYPE_InterfaceAssociation,
		// first interface
		.bFirstInterface = CDC_CONTROL_INTERFACE,
		// number of interfaces
		.bInterfaceCount = 2,
		// set device class to 0x02 "Communications and CDC Control" https://www.usb.org/defined-class-codes
		.bFunctionClass = 0x02,
		// set subclass to 0x02 to automatically load usbser.sys on windows https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/usb-driver-installation-based-on-compatible-ids
		.bFunctionSubClass = 0x02,
		.bFunctionProtocol = 0,
		// no function string
		.iFunction = 0
	},
	
	.CDC_control_interface = {
		.bLength = sizeof(USB_InterfaceDescriptor),
		.bDescriptorType = USB_DTYPE_Interface,
		.bInterfaceNumber = CDC_CONTROL_INTERFACE,
		.bNumEndpoints = 1,
		.bAlternateSetting = 0,
		.bInterfaceClass = CDC_INTERFACE_CLASS,
		.bInterfaceSubClass = CDC_INTERFACE_SUBCLASS_ACM,
		.bInterfaceProtocol = 0,
		.iInterface = 0
	},
	
	.CDC_functional_header = {
		.bLength = sizeof(CDC_FunctionalHeaderDescriptor),
		.bDescriptorType = USB_DTYPE_CSInterface,
		.bDescriptorSubtype = CDC_SUBTYPE_HEADER,
		// version 1.1
		.bcdCDC = 0x0110
	},
	
	.CDC_functional_ACM = {
		.bLength = sizeof(CDC_FunctionalACMDescriptor),
		.bDescriptorType = USB_DTYPE_CSInterface,
		.bDescriptorSubtype = CDC_SUBTYPE_ACM,
		.bmCapabilities = 0x02
	},

	.CDC_functional_union = {
		.bLength = sizeof(CDC_FunctionalUnionDescriptor),
		.bDescriptorType = USB_DTYPE_CSInterface,
		.bDescriptorSubtype = CDC_SUBTYPE_UNION,
		.bMasterInterface = CDC_CONTROL_INTERFACE,
		.bSlaveInterface = CDC_DATA_INTERFACE
	},
	
	.CDC_notification_endpoint = {
		.bLength = sizeof(USB_EndpointDescriptor),
		.bDescriptorType = USB_DTYPE_Endpoint,
		.bEndpointAddress = USB_EP_CDC_NOTIFICATION,
		.bmAttributes = (USB_EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.wMaxPacketSize = 8,
		.bInterval = 0xFF
	},
	
	.CDC_data_interface = {
		.bLength = sizeof(USB_InterfaceDescriptor),
		.bDescriptorType = USB_DTYPE_Interface,
		.bInterfaceNumber = CDC_DATA_INTERFACE,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = CDC_INTERFACE_CLASS_DATA,
		.bInterfaceSubClass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = 0
	},
	
	.CDC_out_endpoint = {
		.bLength = sizeof(USB_EndpointDescriptor),
		.bDescriptorType = USB_DTYPE_Endpoint,
		.bEndpointAddress = USB_EP_CDC_OUT,
		.bmAttributes = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.wMaxPacketSize = USB_SERIAL_BUFFER_SIZE,
		.bInterval = 0x05
	},
	
	.CDC_in_endpoint = {
		.bLength = sizeof(USB_EndpointDescriptor),
		.bDescriptorType = USB_DTYPE_Endpoint,
		.bEndpointAddress = USB_EP_CDC_IN,
		.bmAttributes = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.wMaxPacketSize = USB_SERIAL_BUFFER_SIZE,
		.bInterval = 0x05
	}
};

__attribute__((__aligned__(4))) const USB_StringDescriptor language_string = {
	.bLength = USB_STRING_LEN(1),
	.bDescriptorType = USB_DTYPE_String,
	.bString = {USB_LANGUAGE_EN_US},
};

volatile CDC_LineEncoding line_coding = {
	.dwDTERate = 115000, // baud rate
	.bCharFormat = 0, // 0-1 stop bits
	.bParityType = 0, // no parity
	.bDataBits = 8 // 8 bits per frame
};

volatile uint8_t line_state = 0;

void usb_cb_reset(void) {
	
}

void usb_cb_control_setup(void) {
	if ((usb_setup.bmRequestType & USB_REQTYPE_TYPE_MASK) == USB_REQTYPE_CLASS) {
		uint8_t interface = usb_setup.wIndex & 0xff;
		switch (usb_setup.bmRequestType & USB_REQTYPE_RECIPIENT_MASK) {
			case USB_RECIPIENT_INTERFACE:
			if (interface == CDC_CONTROL_INTERFACE) {
				switch (usb_setup.bRequest) {
					case CDC_SET_LINE_ENCODING:
					for (int i = 0; i < usb_setup.wLength && i < sizeof(CDC_LineEncoding); ++i) {
						((uint8_t*)(&line_coding))[i] = ep0_buf_out[i];
					}
					usb_ep0_in(0);
					usb_ep0_out();
					break;
					
					case CDC_GET_LINE_ENCODING:
					for (int i = 0; i < sizeof(CDC_LineEncoding); ++i) {
						ep0_buf_in[i] = ((uint8_t*)(&line_coding))[i];
					}
					usb_ep0_in(sizeof(CDC_LineEncoding));
					usb_ep0_out();
					break;
					
					case CDC_SET_CONTROL_LINE_STATE:
					line_state = (uint8_t)(usb_setup.wValue);
					usb_ep0_in(0);
					usb_ep0_out();
					break;
					
					case CDC_SEND_BREAK:
					usb_ep0_in(0);
					usb_ep0_out();
					break;
					
					default:
					usb_ep0_stall();
					break;
				}
			}
			break;
			
			//case USB_RECIPIENT_DEVICE:
			//switch (usb_setup.bRequest) {
				//case REQ_PWR:
				//usb_ep0_out();
				//usb_ep0_in(0);
				//break;
				//
				//case REQ_INFO:
				//usb_ep0_in(0);
				//break;
				//
				//case REQ_BOOT:
				//usb_ep0_out();
				//usb_ep0_in(0);
				//break;
				//
				//case REQ_RESET:
				//usb_ep0_out();
				//usb_ep0_in(0);
				//break;
				//
				//case REQ_OPENWRT_BOOT_STATUS:
				//usb_ep0_out();
				//usb_ep0_in(0);
				//break;
				//
				//default:
				//usb_ep0_stall();
				//break;
			//}
			//break;
			
			case USB_RECIPIENT_ENDPOINT:
			default:
			usb_ep0_stall();
			break;
		}
	}
}

void usb_cb_completion(void) {
	if (usb_ep_pending(USB_EP_CDC_OUT)) {
		usb_ep_handled(USB_EP_CDC_OUT);
		usb_serial_out_completion();
	}

	if (usb_ep_pending(USB_EP_CDC_IN)) {
		usb_ep_handled(USB_EP_CDC_IN);
		usb_serial_in_completion();
	}
}

void usb_cb_control_in_completion(void) {
	
}

void usb_cb_control_out_completion(void) {
	
}

bool usb_cb_set_configuration(uint8_t config) {
	if (config <= 1) {
		usb_serial_init();
		return true;
	}
	return false;
}

bool usb_cb_set_interface(uint16_t interface, uint16_t altsetting) {
	switch (interface) {
		case CDC_CONTROL_INTERFACE:
		if (altsetting == 0) return true;
		break;
		case CDC_DATA_INTERFACE:
		if (altsetting == 0) return true;
		break;
	}
	return false;
}

uint16_t usb_cb_get_descriptor(uint8_t type, uint8_t index, const uint8_t** descriptor_ptr) {
	const void* address = 0;
	uint16_t size = 0;
	
	switch (type) {
		case USB_DTYPE_Device:
		address = &device_descriptor;
		size = sizeof(USB_DeviceDescriptor);
		break;
		
		case USB_DTYPE_Configuration:
		address = &full_config;
		size = sizeof(USB_FullConfigurationDescriptor);
		break;
		
		case USB_DTYPE_String:
		switch (index) {
			case 0x00:
			address = &language_string;
			break;
			
			case 0x01:
			address = usb_string_to_descriptor("Finley Blaine");
			break;
			
			case 0x02:
			address = usb_string_to_descriptor("GNC2");
			break;
			
			case 0x03:
			address = samd_serial_number_string_descriptor();
			break;
			
			default:
			*descriptor_ptr = 0;
			return 0;
		}
		size = (((USB_StringDescriptor*)address))->bLength;
		break;
	}
	
	*descriptor_ptr = address;
	return size;
}