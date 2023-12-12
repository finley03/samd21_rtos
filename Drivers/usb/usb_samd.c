#include "usb.h"
#include "usb_samd.h"
#include "usb_samd_internal.h"

#define NVM_USB_PAD_TRANSN_POS  45
#define NVM_USB_PAD_TRANSN_SIZE 5
#define NVM_USB_PAD_TRANSP_POS  50
#define NVM_USB_PAD_TRANSP_SIZE 5
#define NVM_USB_PAD_TRIM_POS  55
#define NVM_USB_PAD_TRIM_SIZE 3

#undef ENABLE

#define USB_GCLK_GEN                    0

void usb_init(){
	uint32_t pad_transn, pad_transp, pad_trim;

	PM_REGS->PM_APBBMASK |= PM_APBBMASK_USB(1);

	GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN(1) |
			GCLK_CLKCTRL_GEN(USB_GCLK_GEN) |
			GCLK_CLKCTRL_ID(USB_GCLK_ID);

	/* Reset */
	USB_REGS->DEVICE.USB_CTRLA = USB_CTRLA_SWRST(1);
	while (USB_REGS->DEVICE.USB_SYNCBUSY & USB_SYNCBUSY_SWRST_Msk);

	USB_REGS->DEVICE.USB_CTRLA = USB_CTRLA_ENABLE(1) | USB_CTRLA_MODE_DEVICE;
	while (USB_REGS->DEVICE.USB_SYNCBUSY & USB_SYNCBUSY_ENABLE_Msk);

	/* Load Pad Calibration */
	// pad_transn = ( *((uint32_t *)(NVMCTRL_OTP4)
	// 		+ (NVM_USB_PAD_TRANSN_POS / 32))
	// 	>> (NVM_USB_PAD_TRANSN_POS % 32))
	// 	& ((1 << NVM_USB_PAD_TRANSN_SIZE) - 1);
	pad_transn = FUSES_OTP4_WORD_1_USB_TRANSN(OTP4_FUSES_REGS->FUSES_OTP4_WORD_1);

	if (pad_transn == 0x1F) {
		pad_transn = 5;
	}

	// pad_transp =( *((uint32_t *)(NVMCTRL_OTP4)
	// 		+ (NVM_USB_PAD_TRANSP_POS / 32))
	// 		>> (NVM_USB_PAD_TRANSP_POS % 32))
	// 		& ((1 << NVM_USB_PAD_TRANSP_SIZE) - 1);
	pad_transp = FUSES_OTP4_WORD_1_USB_TRANSP(OTP4_FUSES_REGS->FUSES_OTP4_WORD_1);

	if (pad_transp == 0x1F) {
		pad_transp = 29;
	}

	// pad_trim =( *((uint32_t *)(NVMCTRL_OTP4)
	// 		+ (NVM_USB_PAD_TRIM_POS / 32))
	// 		>> (NVM_USB_PAD_TRIM_POS % 32))
	// 		& ((1 << NVM_USB_PAD_TRIM_SIZE) - 1);
	pad_trim = FUSES_OTP4_WORD_1_USB_TRIM(OTP4_FUSES_REGS->FUSES_OTP4_WORD_1);

	if (pad_trim == 0x7) {
		pad_trim = 3;
	}

	USB_REGS->DEVICE.USB_PADCAL = USB_PADCAL_TRANSN(pad_transn) | USB_PADCAL_TRANSP(pad_transp) | USB_PADCAL_TRIM(pad_trim);

	memset(usb_endpoints, 0, usb_num_endpoints*sizeof(usb_device_desc_bank_registers_t));
	USB_REGS->DEVICE.USB_DESCADD = (uint32_t)(&usb_endpoints[0]);
	USB_REGS->DEVICE.USB_INTENSET = USB_DEVICE_INTENSET_EORST(1);

	usb_reset();
}

#define USB_EPTYPE_DISABLED 0
#define USB_EPTYPE_CONTROL 1
#define USB_EPTYPE_ISOCHRONOUS 2
#define USB_EPTYPE_BULK 3
#define USB_EPTYPE_INTERRUPT 4
#define USB_EPTYPE_DUAL_BANK 5

void usb_reset(){
	usb_endpoints[0].DEVICE_DESC_BANK[0].USB_ADDR = (uint32_t) &ep0_buf_out;
	// usb_endpoints[0].DEVICE_DESC_BANK[0].PCKSIZE.bit.SIZE=USB_EP_size_to_gc(USB_EP0_SIZE);
	usb_endpoints[0].DEVICE_DESC_BANK[0].USB_PCKSIZE &= ~USB_DEVICE_PCKSIZE_SIZE_Msk;
	usb_endpoints[0].DEVICE_DESC_BANK[0].USB_PCKSIZE |= USB_DEVICE_PCKSIZE_SIZE(USB_EP_size_to_gc(USB_EP0_SIZE));
	usb_endpoints[0].DEVICE_DESC_BANK[1].USB_ADDR = (uint32_t) &ep0_buf_in;
	// usb_endpoints[0].DEVICE_DESC_BANK[1].PCKSIZE.bit.SIZE=USB_EP_size_to_gc(USB_EP0_SIZE);
	// usb_endpoints[0].DEVICE_DESC_BANK[1].PCKSIZE.bit.AUTO_ZLP = 1;
	usb_endpoints[0].DEVICE_DESC_BANK[1].USB_PCKSIZE &= ~USB_DEVICE_PCKSIZE_SIZE_Msk;
	usb_endpoints[0].DEVICE_DESC_BANK[1].USB_PCKSIZE |= USB_DEVICE_PCKSIZE_SIZE(USB_EP_size_to_gc(USB_EP0_SIZE));
	usb_endpoints[0].DEVICE_DESC_BANK[1].USB_PCKSIZE &= ~USB_DEVICE_PCKSIZE_AUTO_ZLP_Msk;
	usb_endpoints[0].DEVICE_DESC_BANK[1].USB_PCKSIZE |= USB_DEVICE_PCKSIZE_AUTO_ZLP(1);
	USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTENSET = USB_DEVICE_EPINTENSET_RXSTP(1);
	USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPCFG = USB_DEVICE_EPCFG_EPTYPE0(USB_EPTYPE_CONTROL)
	                                         | USB_DEVICE_EPCFG_EPTYPE1(USB_EPTYPE_CONTROL);
}

void usb_set_address(uint8_t addr) {
	USB_REGS->DEVICE.USB_DADD = USB_DEVICE_DADD_ADDEN(1) | addr;
}

inline usb_device_desc_bank_registers_t* ep_ram(uint8_t epaddr) {
	return &usb_endpoints[epaddr&0x3F].DEVICE_DESC_BANK[!!(epaddr&0x80)];
}

inline void usb_enable_ep(uint8_t ep, uint8_t type, usb_size bufsize) {
	if (ep & 0x80) {
		// usb_endpoints[ep & 0x3f].DEVICE_DESC_BANK[1].PCKSIZE.bit.SIZE = USB_EP_size_to_gc(bufsize);
		usb_endpoints[ep & 0x3f].DEVICE_DESC_BANK[1].USB_PCKSIZE &= ~USB_DEVICE_PCKSIZE_SIZE_Msk;
		usb_endpoints[ep & 0x3f].DEVICE_DESC_BANK[1].USB_PCKSIZE |= USB_DEVICE_PCKSIZE_SIZE(USB_EP_size_to_gc(bufsize));
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3f].USB_EPCFG |= USB_DEVICE_EPCFG_EPTYPE1(type + 1);
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3f].USB_EPSTATUSCLR = USB_DEVICE_EPSTATUS_BK1RDY(1)
		                                                      | USB_DEVICE_EPSTATUS_STALLRQ(0x2)
		                                                      | USB_DEVICE_EPSTATUS_DTGLIN(1);
	} else {
		// usb_endpoints[ep & 0x3f].DEVICE_DESC_BANK[0].PCKSIZE.bit.SIZE = USB_EP_size_to_gc(bufsize);
		usb_endpoints[ep & 0x3f].DEVICE_DESC_BANK[0].USB_PCKSIZE &= ~USB_DEVICE_PCKSIZE_SIZE_Msk;
		usb_endpoints[ep & 0x3f].DEVICE_DESC_BANK[0].USB_PCKSIZE |= USB_DEVICE_PCKSIZE_SIZE(USB_EP_size_to_gc(bufsize));
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3f].USB_EPCFG |= USB_DEVICE_EPCFG_EPTYPE0(type + 1);
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3f].USB_EPSTATUSSET = USB_DEVICE_EPSTATUS_BK0RDY(1);
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3f].USB_EPSTATUSCLR = USB_DEVICE_EPSTATUS_STALLRQ(0x1)
		                                                      | USB_DEVICE_EPSTATUS_DTGLOUT(1);
	}
}

inline void usb_disable_ep(uint8_t ep) {
	if (ep & 0x80) {
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3f].USB_EPSTATUSCLR = USB_DEVICE_EPSTATUS_BK1RDY(1);
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3f].USB_EPCFG |= USB_DEVICE_EPCFG_EPTYPE1(USB_EPTYPE_DISABLED);
	} else {
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3f].USB_EPSTATUSSET = USB_DEVICE_EPSTATUS_BK0RDY(1);
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3f].USB_EPCFG |= USB_DEVICE_EPCFG_EPTYPE0(USB_EPTYPE_DISABLED);
	}
}

inline void usb_reset_ep(uint8_t ep){
	if (ep & 0x80) {
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3f].USB_EPSTATUSCLR = USB_DEVICE_EPSTATUS_BK1RDY(1);
	} else {
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3f].USB_EPSTATUSSET = USB_DEVICE_EPSTATUS_BK0RDY(1);
	}
}

inline usb_bank usb_ep_start_out(uint8_t ep, uint8_t* data, usb_size len) {
	// usb_endpoints[ep].DEVICE_DESC_BANK[0].PCKSIZE.bit.MULTI_PACKET_SIZE = len;
	usb_endpoints[ep].DEVICE_DESC_BANK[0].USB_PCKSIZE &= ~USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE_Msk;
	usb_endpoints[ep].DEVICE_DESC_BANK[0].USB_PCKSIZE |= USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE(len);
	// usb_endpoints[ep].DEVICE_DESC_BANK[0].PCKSIZE.bit.BYTE_COUNT = 0;
	usb_endpoints[ep].DEVICE_DESC_BANK[0].USB_PCKSIZE &= ~USB_DEVICE_PCKSIZE_BYTE_COUNT_Msk;
	usb_endpoints[ep].DEVICE_DESC_BANK[0].USB_PCKSIZE |= USB_DEVICE_PCKSIZE_BYTE_COUNT(0);
	usb_endpoints[ep].DEVICE_DESC_BANK[0].USB_ADDR = (uint32_t) data;
	USB_REGS->DEVICE.DEVICE_ENDPOINT[ep].USB_EPINTFLAG = USB_DEVICE_EPINTFLAG_TRCPT0(1) | USB_DEVICE_EPINTFLAG_TRFAIL0(1);
	USB_REGS->DEVICE.DEVICE_ENDPOINT[ep].USB_EPINTENSET = USB_DEVICE_EPINTENSET_TRCPT0(1);
	USB_REGS->DEVICE.DEVICE_ENDPOINT[ep].USB_EPSTATUSCLR = USB_DEVICE_EPSTATUS_BK0RDY(1);
	return 0;
}

inline usb_bank usb_ep_start_in(uint8_t ep, const uint8_t* data, usb_size size, bool zlp) {
	ep &= 0x3f;
	// usb_endpoints[ep].DEVICE_DESC_BANK[1].PCKSIZE.bit.AUTO_ZLP = zlp;
	usb_endpoints[ep].DEVICE_DESC_BANK[1].USB_PCKSIZE &= ~USB_DEVICE_PCKSIZE_AUTO_ZLP_Msk;
	usb_endpoints[ep].DEVICE_DESC_BANK[1].USB_PCKSIZE |= USB_DEVICE_PCKSIZE_AUTO_ZLP(zlp);
	usb_endpoints[ep].DEVICE_DESC_BANK[1].USB_PCKSIZE &= ~USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE_Msk;
	usb_endpoints[ep].DEVICE_DESC_BANK[1].USB_PCKSIZE |= USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE(0);
	usb_endpoints[ep].DEVICE_DESC_BANK[1].USB_PCKSIZE &= ~USB_DEVICE_PCKSIZE_BYTE_COUNT_Msk;
	usb_endpoints[ep].DEVICE_DESC_BANK[1].USB_PCKSIZE |= USB_DEVICE_PCKSIZE_BYTE_COUNT(size);
	usb_endpoints[ep].DEVICE_DESC_BANK[1].USB_ADDR = (uint32_t) data;
	USB_REGS->DEVICE.DEVICE_ENDPOINT[ep].USB_EPINTFLAG = USB_DEVICE_EPINTFLAG_TRCPT1(1) | USB_DEVICE_EPINTFLAG_TRFAIL1(1);
	USB_REGS->DEVICE.DEVICE_ENDPOINT[ep].USB_EPINTENSET = USB_DEVICE_EPINTENSET_TRCPT1(1);
	USB_REGS->DEVICE.DEVICE_ENDPOINT[ep].USB_EPSTATUSSET = USB_DEVICE_EPSTATUS_BK1RDY(1);
	return 0;
}

inline bool usb_ep_empty(uint8_t ep) {
	if (ep & 0x80) {
		return !((USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3F].USB_EPSTATUS & USB_DEVICE_EPSTATUS_BK1RDY_Msk) || usb_ep_pending(ep));
	} else {
		return !((USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3F].USB_EPSTATUS & USB_DEVICE_EPSTATUS_BK0RDY_Msk) || usb_ep_pending(ep));
	}
}

inline bool usb_ep_ready(uint8_t ep) {
	return usb_ep_empty(ep);
}

inline bool usb_ep_pending(uint8_t ep) {
	if (ep & 0x80) {
		return USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3F].USB_EPINTFLAG & USB_DEVICE_EPINTFLAG_TRCPT1_Msk;
	} else {
		return USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3F].USB_EPINTFLAG & USB_DEVICE_EPINTFLAG_TRCPT0_Msk;
	}
}

inline void usb_ep_handled(uint8_t ep) {
	if (ep & 0x80) {
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3F].USB_EPINTFLAG = USB_DEVICE_EPINTFLAG_TRCPT1(1);
	} else {
		USB_REGS->DEVICE.DEVICE_ENDPOINT[ep & 0x3F].USB_EPINTFLAG = USB_DEVICE_EPINTFLAG_TRCPT0(1);
	}
}

inline usb_size usb_ep_out_length(uint8_t ep){
	return (usb_endpoints[ep].DEVICE_DESC_BANK[0].USB_PCKSIZE & USB_DEVICE_PCKSIZE_BYTE_COUNT_Msk) >> USB_DEVICE_PCKSIZE_BYTE_COUNT_Pos;
}

inline void usb_detach(void) {
	USB_REGS->DEVICE.USB_CTRLB |= USB_DEVICE_CTRLB_DETACH(1);
	NVIC_DisableIRQ(USB_IRQn);
}

inline void usb_attach(void) {
	NVIC_EnableIRQ(USB_IRQn);
	USB_REGS->DEVICE.USB_CTRLB &= ~USB_DEVICE_CTRLB_DETACH(1);
}

/// Enable the OUT stage on the default control pipe.
inline void usb_ep0_out(void) {
	usb_ep_start_out(0x00, ep0_buf_out, USB_EP0_SIZE);
}

inline void usb_ep0_in(uint8_t size){
	usb_ep_start_in(0x80, ep0_buf_in, size, true);
}

inline void usb_ep0_stall(void) {
	USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPSTATUSSET = USB_DEVICE_EPSTATUS_STALLRQ(0x3);
}

void usb_set_speed(USB_Speed speed) {
	if (USB_SPEED_FULL == speed) {
		//USB->DEVICE.CTRLB.bit.SPDCONF = USB_DEVICE_CTRLB_SPDCONF_0_Val;
		// USB_REGS->DEVICE.CTRLB.bit.SPDCONF = USB_DEVICE_CTRLB_SPDCONF_FS_Val;
		USB_REGS->DEVICE.USB_CTRLB &= ~USB_DEVICE_CTRLB_SPDCONF_Msk;
		USB_REGS->DEVICE.USB_CTRLB |= USB_DEVICE_CTRLB_SPDCONF_FS;
	} else if(USB_SPEED_LOW == speed) {
		//USB->DEVICE.CTRLB.bit.SPDCONF = USB_DEVICE_CTRLB_SPDCONF_1_Val;
		// USB_REGS->DEVICE.CTRLB.bit.SPDCONF = USB_DEVICE_CTRLB_SPDCONF_LS_Val;
		USB_REGS->DEVICE.USB_CTRLB &= ~USB_DEVICE_CTRLB_SPDCONF_Msk;
		USB_REGS->DEVICE.USB_CTRLB |= USB_DEVICE_CTRLB_SPDCONF_LS;
	}
}
USB_Speed usb_get_speed() {
	if (!(USB_REGS->DEVICE.USB_STATUS & USB_DEVICE_STATUS_SPEED_Msk)) {
		return USB_SPEED_LOW;
	} else {
		return USB_SPEED_FULL;
	}
}

//void usb_handle_function() {
	//uint32_t summary = USB->DEVICE.EPINTSMRY.reg;
	//uint32_t status = USB->DEVICE.INTFLAG.reg;
//
	//if (status & USB_DEVICE_INTFLAG_EORST) {
		//USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_EORST;
		//usb_reset();
		//usb_cb_reset();
		//return;
	//}
//
	//if (summary & (1<<0)) {
		//uint32_t flags = USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg;
		//USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT1 | USB_DEVICE_EPINTFLAG_TRCPT0 | USB_DEVICE_EPINTFLAG_RXSTP;
		//if (flags & USB_DEVICE_EPINTFLAG_RXSTP) {
			//memcpy(&usb_setup, ep0_buf_out, sizeof(usb_setup));
			//usb_handle_setup();
		//}
		//if (flags & USB_DEVICE_EPINTFLAG_TRCPT0) {
			//usb_handle_control_out_complete();
		//}
		//if (flags & USB_DEVICE_EPINTFLAG_TRCPT1) {
			//usb_handle_control_in_complete();
		//}
	//}
//
	//for (int i=1; i<usb_num_endpoints; i++) {
		//if (summary & 1<<i) {
			//uint32_t flags = USB->DEVICE.DeviceEndpoint[i].EPINTFLAG.reg;
			//USB->DEVICE.DeviceEndpoint[i].EPINTENCLR.reg = flags;
		//}
	//}
//
	//usb_cb_completion();
//}

void usb_handle_function() {
	uint32_t summary = USB_REGS->DEVICE.USB_EPINTSMRY;
	uint32_t status = USB_REGS->DEVICE.USB_INTFLAG;

	if (status & USB_DEVICE_INTFLAG_EORST(1)) {
		USB_REGS->DEVICE.USB_INTFLAG = USB_DEVICE_INTFLAG_EORST(1);
		usb_reset();
		usb_cb_reset();
		goto usb_handle_function_end;
	}

	if (summary & (1<<0)) {
		uint32_t flags = USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTFLAG;
		USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTFLAG = USB_DEVICE_EPINTFLAG_TRCPT1(1) | USB_DEVICE_EPINTFLAG_TRCPT0(1) | USB_DEVICE_EPINTFLAG_RXSTP(1);
		if (flags & USB_DEVICE_EPINTFLAG_RXSTP(1)) {
			memcpy(&usb_setup, ep0_buf_out, sizeof(usb_setup));
			usb_handle_setup();
		}
		if (flags & USB_DEVICE_EPINTFLAG_TRCPT0(1)) {
			usb_handle_control_out_complete();
		}
		if (flags & USB_DEVICE_EPINTFLAG_TRCPT1(1)) {
			usb_handle_control_in_complete();
		}
	}

	for (int i=1; i<usb_num_endpoints; i++) {
		if (summary & 1<<i) {
			uint32_t flags = USB_REGS->DEVICE.DEVICE_ENDPOINT[i].USB_EPINTFLAG;
			USB_REGS->DEVICE.DEVICE_ENDPOINT[i].USB_EPINTENCLR = flags;
		}
	}

	usb_cb_completion();
	
	usb_handle_function_end:
	NVIC_EnableIRQ(USB_IRQn);
}

void* samd_serial_number_string_descriptor() {
	char buf[27];

	const unsigned char* id = (unsigned char*) 0x0080A00C;
	for (int i=0; i<26; i++) {
		unsigned idx = (i*5)/8;
		unsigned pos = (i*5)%8;
		unsigned val = ((id[idx] >> pos) | (id[idx+1] << (8-pos))) & ((1<<5)-1);
		buf[i] = "0123456789ABCDFGHJKLMNPQRSTVWXYZ"[val];
	}
	buf[26] = 0;
	return usb_string_to_descriptor(buf);
}
