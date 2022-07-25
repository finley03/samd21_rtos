#ifndef USB_H
#define USB_H

typedef uint_least16_t char16_t;

// usb vid pid
// #define USB_UNIQUE_VID_PID

#define USB_VID 0x1209
#define USB_PID 0x000E

#ifndef USB_UNIQUE_VID_PID
#warning The USB VID/PID pair is NOT unique and this product should not be used outside a testing environment
#endif

// usb descriptors

typedef enum {
	USB_DescriptorType_DEVICE = 1,
	USB_DescriptorType_CONFIGURATION = 2,
	USB_DescriptorType_STRING = 3,
	USB_DescriptorType_INTERFACE = 4,
	USB_DescriptorType_ENDPOINT = 5,
	USB_DescriptorType_DEVICE_QUALIFIER = 6,
	USB_DescriptorType_OTHER_SPEED_CONFIGURATION = 7,
	USB_DescriptorType_INTERFACE_POWER = 8,
} USB_DescriptorType;

typedef struct __attribute__((packed)) {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
} USB_DeviceDescriptor;

typedef struct __attribute__((packed)) {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
} USB_ConfigurationDescriptor;

typedef struct __attribute__((packed)) {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} USB_InterfaceDescriptor;

typedef struct __attribute__((packed)) {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
} USB_EndpointDescriptor;

typedef struct __attribute__((packed)) {
	uint8_t bLength;
	uint8_t bDescriptorType;
	char16_t bString[];
} USB_StringDescriptor;

#define USB_StringDescriptor_bLength(len) (2 + (len * 2))

typedef struct __attribute__((packed)) {
	USB_ConfigurationDescriptor configuration_descriptor;
	USB_InterfaceDescriptor interface_descriptor;
	//USB_EndpointDescriptor in_endpoint_descriptor;
	//USB_EndpointDescriptor out_endpoint_descriptor;
} USB_FullConfigurationDescriptor;


typedef enum {
	USB_bmRequestType_Direction_hosttodevice = 0,
	USB_bmRequestType_Direction_devicetohost = 1,
} USB_bmRequestType_Direction_Val;
#define USB_bmRequestType_Direction_Offset 7

typedef enum {
	USB_bmRequestType_Type_standard = 0,
	USB_bmRequestType_Type_class = 1,
	USB_bmRequestType_Type_vendor = 2,
} USB_bmRequestType_Type_Val;
#define USB_bmRequestType_Type_Offset 5

typedef enum {
	USB_bmRequestType_Recipient_device = 0,
	USB_bmRequestType_Recipient_interface = 1,
	USB_bmRequestType_Recipient_endpoint = 2,
	USB_bmRequestType_Recipient_other = 3,
} USB_bmRequestType_Recipient_Val;
#define USB_bmRequestType_Recipient_Offset 0

typedef enum {
	USB_bRequest_GET_STATUS = 0,
	USB_bRequest_CLEAR_FEATURE = 1,
	USB_bRequest_SET_FEATURE = 3,
	USB_bRequest_SET_ADDRESS = 5,
	USB_bRequest_GET_DESCRIPTOR = 6,
	USB_bRequest_SET_DESCRIPTOR = 7,
	USB_bRequest_GET_CONFIGURATION = 8,
	USB_bRequest_SET_CONFIGURATION = 9,
	USB_bRequest_GET_INTERFACE = 10,
	USB_bRequest_SET_INTERFACE = 11,
	USB_bRequest_SYNCH_FRAME = 12,
} USB_bRequest;

typedef struct __attribute__((packed)) {
	union {
		struct {
			uint8_t recipient : 5;
			uint8_t type : 2;
			uint8_t direction : 1;
		} bit;
		uint8_t reg;
	} bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} USB_SetupPacket;

void usb_init();
void usb_reset_endpoints();
void usb_attach();
void usb_detatch();

#endif