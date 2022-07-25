#include "samd21.h"
#include "usb.h"
#include "port.h"
#include <string.h>
#include <stdbool.h>

extern bool interrupted;

// address of memory area containing calibration values
#define NVM_CAL_ADDR 0x00806020

// number of usb endpoints
#define NR_USB_ENDPOINTS 1

#define EP0_SIZE 64

// usb endpoint descriptor data structure
USB_EndpointDescriptor usb_endpoints[NR_USB_ENDPOINTS];

UsbDeviceDescriptor usb_endpoints_internal[NR_USB_ENDPOINTS];

uint8_t __attribute__((aligned(4))) ep0_in[EP0_SIZE];
uint8_t __attribute__((aligned(4))) ep0_out[EP0_SIZE];

USB_SetupPacket usb_setup;

volatile uint8_t usb_configuration;

bool getDeviceDescriptor;

const USB_DeviceDescriptor device_descriptor = {
	.bLength = sizeof(USB_DeviceDescriptor) - 1,
	.bDescriptorType = USB_DescriptorType_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0xFF,
	.bDeviceSubClass = 0xFF,
	.bDeviceProtocol = 0xFF,
	.bMaxPacketSize = 64,
	.idVendor = USB_VID,
	.idProduct = USB_PID,
	.bcdDevice = 0x0100,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 0,
	.bNumConfigurations = 1,
};


const USB_FullConfigurationDescriptor full_configuration_descriptor = {
	.configuration_descriptor = {
		.bLength = sizeof(USB_ConfigurationDescriptor),
		.bDescriptorType = USB_DescriptorType_CONFIGURATION,
		.wTotalLength = sizeof(USB_FullConfigurationDescriptor),
		.bNumInterfaces = 1,
		.bConfigurationValue = 0,
		.iConfiguration = 0,
		.bmAttributes = 0b10000000,
		.bMaxPower = 50,
	},
	.interface_descriptor = {
		.bLength = sizeof(USB_InterfaceDescriptor),
		.bDescriptorType = USB_DescriptorType_INTERFACE,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0,
		.bInterfaceClass = 0xFF,
		.bInterfaceSubClass = 0xFF,
		.bInterfaceProtocol = 0xFF,
		.iInterface = 0,
	},
	//.in_endpoint_descriptor = {
		//
	//},
	//.out_endpoint_descriptor = {
		//
	//},
};


const USB_StringDescriptor language_string = {
	.bLength = USB_StringDescriptor_bLength(1),
	.bDescriptorType = USB_DescriptorType_STRING,
	.bString = {0x0409},
};

const USB_StringDescriptor manufacturer_string = {
	.bLength = USB_StringDescriptor_bLength(13),
	.bDescriptorType = USB_DescriptorType_STRING,
	.bString = u"Finley Blaine",
};

const USB_StringDescriptor device_string = {
	.bLength = USB_StringDescriptor_bLength(18),
	.bDescriptorType = USB_DescriptorType_STRING,
	.bString = u"RTOS_USB_INTERFACE",
};


void usb_init() {
	// enable comms bus for USB
	PM->APBBMASK.bit.USB_ = 1;
	
	// enable clock for USB FOR REVIEW
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_USB | GCLK_CLKCTRL_GEN(0);
	
	// reset usb
	USB->DEVICE.CTRLA.bit.SWRST = 1;
	while (USB->DEVICE.SYNCBUSY.bit.SWRST);
	
	// configure ports
	PORT_WRCONFIG_Type wrconfig = {
		// upper 16 bits
		.bit.HWSEL = 1,
		
		// enable update pincfg
		.bit.WRPINCFG = 1,
		
		// enable update of pmux
		.bit.WRPMUX = 1,
		
		// pin multiplexing function G
		.bit.PMUX = 6,
		
		// enable pin multiplexing
		.bit.PMUXEN = 1,
		
		// select pins
		.bit.PINMASK = (uint16_t)((PORT_PA24 | PORT_PA25) >> 16)
	};
	
	// apply wrconfig data structure
	PORT->Group[0].WRCONFIG.reg = wrconfig.reg;
	
	// get USB PADCAL values
	uint64_t data = *(uint64_t*)(NVM_CAL_ADDR);
	
	// extract values needed from data
	uint8_t usb_transn = (data >> 45) & 0b00011111;
	uint8_t usb_transp = (data >> 50) & 0b00011111;
	uint8_t usb_trim = (data >> 55) & 0b00000111;
	
	// set values in calibration registers
	USB->DEVICE.PADCAL.bit.TRANSN = usb_transn;
	USB->DEVICE.PADCAL.bit.TRANSP = usb_transp;
	USB->DEVICE.PADCAL.bit.TRIM = usb_trim;
	
	// set USB to device mode
	USB->DEVICE.CTRLA.bit.MODE = 0;
	// set USB speed to full
	USB->DEVICE.CTRLB.bit.SPDCONF = 0;
	
	// enable USB
	USB->DEVICE.CTRLA.bit.ENABLE = 1;
	while (USB->DEVICE.SYNCBUSY.bit.ENABLE);
	
	// set memory space for USB endpoint descriptors
	memset(usb_endpoints_internal, 0, sizeof(UsbDeviceDescriptor) * NR_USB_ENDPOINTS);
	// tell usb controller start address of usb endpoints
	USB->DEVICE.DESCADD.reg = (uint32_t)usb_endpoints_internal;
	// enable interrupts for end of reset
	USB->DEVICE.INTENSET.bit.EORST = 1;
	
	usb_reset_endpoints();
	
	usb_configuration = 0;
	getDeviceDescriptor = false;
}


void usb_reset_endpoints() {
	// configure control out endpoint
	USB->DEVICE.DeviceEndpoint[0].EPCFG.bit.EPTYPE0 = 0x1;
	// configure control in endpoint
	USB->DEVICE.DeviceEndpoint[0].EPCFG.bit.EPTYPE1 = 0x1;
	
	// configure endpoint size to 64 bytes
	usb_endpoints_internal[0].DeviceDescBank[0].PCKSIZE.bit.SIZE = 0x3;
	usb_endpoints_internal[0].DeviceDescBank[1].PCKSIZE.bit.SIZE = 0x3;
	usb_endpoints_internal[0].DeviceDescBank[0].ADDR.reg = (uint32_t)&ep0_out;
	usb_endpoints_internal[0].DeviceDescBank[1].ADDR.reg = (uint32_t)&ep0_in;
	usb_endpoints_internal[0].DeviceDescBank[1].PCKSIZE.bit.AUTO_ZLP = 1;
	
	// enable received setup interrupt enable
	USB->DEVICE.DeviceEndpoint[0].EPINTENSET.bit.RXSTP = 1;
}


void usb_attach() {
	// enable USB interrupt requests
	NVIC_EnableIRQ(USB_IRQn);
	USB->DEVICE.CTRLB.bit.DETACH = 0;
}


void usb_detatch() {
	USB->DEVICE.CTRLB.bit.DETACH = 1;
	// disable USB interrupts
	NVIC_DisableIRQ(USB_IRQn);
}


void usb_ep_out(uint8_t endpoint, uint8_t* data, int length) {
	usb_endpoints_internal[endpoint].DeviceDescBank[0].PCKSIZE.bit.MULTI_PACKET_SIZE = length;
	usb_endpoints_internal[endpoint].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT = 0;
	usb_endpoints_internal[endpoint].DeviceDescBank[0].ADDR.reg = (uint32_t)data;
	USB->DEVICE.DeviceEndpoint[endpoint].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT0 | USB_DEVICE_EPINTFLAG_TRFAIL0;
	USB->DEVICE.DeviceEndpoint[endpoint].EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT0;
	USB->DEVICE.DeviceEndpoint[endpoint].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUS_BK0RDY;
}

void usb_ep_in(uint8_t endpoint, uint8_t* data, int length) {
	usb_endpoints_internal[endpoint].DeviceDescBank[1].PCKSIZE.bit.AUTO_ZLP = 1;
	usb_endpoints_internal[endpoint].DeviceDescBank[1].PCKSIZE.bit.MULTI_PACKET_SIZE = 0;
	usb_endpoints_internal[endpoint].DeviceDescBank[1].PCKSIZE.bit.BYTE_COUNT = length;
	usb_endpoints_internal[endpoint].DeviceDescBank[1].ADDR.reg = (uint32_t)data;
	USB->DEVICE.DeviceEndpoint[endpoint].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT1 | USB_DEVICE_EPINTFLAG_TRFAIL1;
	USB->DEVICE.DeviceEndpoint[endpoint].EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT1;
	USB->DEVICE.DeviceEndpoint[endpoint].EPSTATUSSET.reg = USB_DEVICE_EPSTATUS_BK1RDY;
}

#define usb_ep0_out() usb_ep_out(0, ep0_out, EP0_SIZE)
#define usb_ep0_in(length) usb_ep_in(0, ep0_in, length)

void* usb_get_descriptor(uint16_t* size) {
	interrupted = true;
	
	uint8_t type = (usb_setup.wValue >> 8);
	uint8_t index = (usb_setup.wValue & 0xFF);
	
	switch (type) {
		case USB_DescriptorType_DEVICE:
		*size = sizeof(USB_DeviceDescriptor);
		getDeviceDescriptor = true;
		return &device_descriptor;
		break;
		
		case USB_DescriptorType_CONFIGURATION:
		*size = sizeof(USB_FullConfigurationDescriptor);
		return &full_configuration_descriptor;
		break;
		
		case USB_DescriptorType_STRING:
		switch (index) {
			case 0:
			*size = language_string.bLength;
			return &language_string;
			break;
			
			case 1:
			*size = manufacturer_string.bLength;
			return &manufacturer_string;
			break;
			
			case 2:
			*size = device_string.bLength;
			return &device_string;
			break;
			
			default:
			*size = 0;
			return 0;
			break;
		}
		break;
		
		default:
		*size = 0;
		return 0;
		break;
	}
}


void usb_handle_setup_packet() {
	if (usb_setup.bmRequestType.bit.type == USB_bmRequestType_Type_standard) {
		switch (usb_setup.bRequest) {
			case USB_bRequest_GET_STATUS:
			ep0_in[0] = 0;
			ep0_in[1] = 0;
			usb_ep0_in(2);
			usb_ep0_out();
			break;
			
			case USB_bRequest_CLEAR_FEATURE:
			usb_ep0_in(0);
			usb_ep0_out();
			break;
			
			case USB_bRequest_SET_FEATURE:
			usb_ep0_in(0);
			usb_ep0_out();
			break;
			
			case USB_bRequest_SET_ADDRESS:
			usb_ep0_in(0);
			usb_ep0_out();
			break;
			
			case USB_bRequest_GET_DESCRIPTOR:
			{
				uint16_t size;
				void* descriptor = usb_get_descriptor(&size);
				
				if (size && descriptor) {
					if (size > usb_setup.wLength) size = usb_setup.wLength;
					
					memcpy(ep0_in, descriptor, size);
					usb_ep0_in(size);
					//usb_ep0_out();
				}
			}
			break;
			
			case USB_bRequest_SET_DESCRIPTOR:
			break;
			
			case USB_bRequest_GET_CONFIGURATION:
			ep0_in[0] = usb_configuration;
			usb_ep0_in(1);
			usb_ep0_out();
			break;
			
			case USB_bRequest_SET_CONFIGURATION:
			usb_ep0_in(0);
			usb_ep0_out();
			break;
			
			case USB_bRequest_GET_INTERFACE:
			break;
			
			case USB_bRequest_SET_INTERFACE:
			usb_ep0_in(0);
			usb_ep0_out();
			break;
			
			case USB_bRequest_SYNCH_FRAME:
			break;
			
			default:
			break;			
		}
	}
}

void USB_Handler() {
	// usb interrupt flags
	uint16_t intflags = USB->DEVICE.INTFLAG.reg;
	// summary of which endpoints have interrupts
	uint16_t epsummary = USB->DEVICE.EPINTSMRY.reg;
	
	// for end of reset interrupt
	if (intflags & USB_DEVICE_INTFLAG_EORST) {
		// clear interrupt
		USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_EORST;
		usb_reset_endpoints();
		return;
	}
	
	if (getDeviceDescriptor) {
		interrupted = true;
	}
	
	// check endpoint 0
	if (epsummary & USB_DEVICE_EPINTSMRY_EPINT0) {
		// get interrupt flags for endpoint 0
		uint8_t flags = USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg;
		USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_RXSTP | USB_DEVICE_EPINTFLAG_TRCPT0 | USB_DEVICE_EPINTFLAG_TRCPT1;
		
		// check receive setup command
		if (flags & USB_DEVICE_EPINTFLAG_RXSTP) {
			// copy data from endpoint to usb_setup
			memcpy(&usb_setup, ep0_out, sizeof(USB_SetupPacket));
			// handle the setup packet
			usb_handle_setup_packet();
		}
		// handle control out complete
		if (flags & USB_DEVICE_EPINTFLAG_TRCPT0) {
			
		}
		// handle control in complete
		if (flags & USB_DEVICE_EPINTFLAG_TRCPT1) {
			if (usb_setup.bmRequestType.bit.type == USB_bmRequestType_Type_standard) {
				if (usb_setup.bRequest == USB_bRequest_SET_ADDRESS) {
					USB->DEVICE.DADD.bit.DADD = usb_setup.wValue;
					usb_ep0_out();
					return;
				}
			}
		}
	}
}