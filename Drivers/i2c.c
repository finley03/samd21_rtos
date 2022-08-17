#include "i2c.h"
#include "sercom.h"

typedef enum {
	I2C_Status_Idle,
	I2C_Status_AddressWrite,
	I2C_Status_AddressRead,
	I2C_Status_DataWrite,
	I2C_Status_DataRead,
	I2C_Status_RepeatStart,
} I2C_Status;

typedef enum {
	I2C_Command_NOACT,
	I2C_Command_ACK_RepeatStart,
	I2C_Command_ACK_ReadByte,
	I2C_Command_ACK_Stop,
	
} I2C_Command;

I2C_Status status;

bool i2c_init(Sercom* sercom) {
	if (!sercom_init(sercom)) return false;
	
	// set mode to master, standard / fast mode, 
	sercom->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_MODE_I2C_MASTER | SERCOM_I2CM_CTRLA_SPEED(0);
	
	// set baud
	sercom->I2CM.BAUD.reg = SERCOM_I2CM_BAUD_BAUD(32);
	
	// enable
	sercom->I2CM.CTRLA.bit.ENABLE = 1;
	while (sercom->I2CM.SYNCBUSY.bit.ENABLE);
	
	// force bus state to idle
	sercom->I2CM.STATUS.reg = SERCOM_I2CM_STATUS_BUSSTATE(1);
	while (sercom->I2CM.SYNCBUSY.bit.SYSOP);
	
	sercom->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_MB | SERCOM_I2CM_INTENSET_SB | SERCOM_I2CM_INTENSET_ERROR;
	NVIC_EnableIRQ(SERCOM0_IRQn);
		
	status = I2C_Status_Idle;
	
	return true;
}

uint8_t* buffer;
int nr_bytes;
int bytes_sent;
int bytes_received;

void i2c_send_data(Sercom* sercom, uint8_t device_address, uint8_t* data, int count) {
	//while (status != I2C_Status_Idle);
	while (SERCOM0->I2CM.STATUS.bit.BUSSTATE != 0x1);
	
	buffer = data;
	nr_bytes = count;
	bytes_sent = 0;
	sercom->I2CM.ADDR.bit.ADDR = device_address << 1;
	while (sercom->I2CM.SYNCBUSY.bit.SYSOP);
	
	//while (status != I2C_Status_Idle);
	while (SERCOM0->I2CM.STATUS.bit.BUSSTATE != 0x1);
}

void i2c_receive_data(Sercom* sercom, uint8_t device_address, uint8_t* data, int count) {
	//while (status != I2C_Status_Idle);
	while (SERCOM0->I2CM.STATUS.bit.BUSSTATE != 0x1);
	
	buffer = data;
	nr_bytes = count;
	bytes_received = 0;
	sercom->I2CM.ADDR.bit.ADDR = (device_address << 1) | 0x1;
	while (sercom->I2CM.SYNCBUSY.bit.SYSOP);
	
	//while (status != I2C_Status_Idle);
	while (SERCOM0->I2CM.STATUS.bit.BUSSTATE != 0x1);
}

//uint8_t i2c_read_byte(Sercom* sercom, uint8_t device_address, uint8_t address) {
	//sercom->I2CM.ADDR.bit.ADDR = (device_address << 1) | 1;
	//while (sercom->I2CM.SYNCBUSY.bit.SYSOP);
	//sercom->I2CM.DATA.reg = address;
	//while (sercom->I2CM.SYNCBUSY.bit.SYSOP);
	//volatile uint8_t data;
	//data = sercom->I2CM.DATA.reg;
	//while (sercom->I2CM.SYNCBUSY.bit.SYSOP);
	//return data;
//}

void SERCOM0_Handler() {
	volatile uint8_t flags = SERCOM0->I2CM.INTFLAG.reg;
	volatile uint16_t status = SERCOM0->I2CM.STATUS.reg;
	
	if (flags & SERCOM_I2CM_INTFLAG_MB) {
		// if no ack packet received
		if (status & SERCOM_I2CM_STATUS_RXNACK) {
			// send stop condition
			SERCOM0->I2CM.CTRLB.bit.CMD = I2C_Command_ACK_Stop;
			// clear interrupts
			SERCOM0->I2CM.INTFLAG.reg = flags;
			return;
		}
		// else send data
		if (bytes_sent < nr_bytes) {
			SERCOM0->I2CM.DATA.bit.DATA = buffer[bytes_sent++];
			// clear interrupts
			SERCOM0->I2CM.INTFLAG.reg = flags;
			return;
		}
		else {
			// send stop condition
			SERCOM0->I2CM.CTRLB.bit.CMD = I2C_Command_ACK_Stop;
			// clear interrupts
			SERCOM0->I2CM.INTFLAG.reg = flags;
			return;
		}
	}
	if (flags & SERCOM_I2CM_INTFLAG_SB) {
		// check if more bytes to come
		if (bytes_received + 1 < nr_bytes) {
			// if so, read data
			buffer[bytes_received++] = SERCOM0->I2CM.DATA.bit.DATA;
			// send ack
			SERCOM0->I2CM.CTRLB.bit.CMD = I2C_Command_ACK_ReadByte;
			// clear interrupts
			SERCOM0->I2CM.INTFLAG.reg = flags;
			return;
		}
		else {
			// read data
			buffer[bytes_received++] = SERCOM0->I2CM.DATA.bit.DATA;
			// send stop condition
			SERCOM0->I2CM.CTRLB.bit.CMD = I2C_Command_ACK_Stop;
			// clear interrupts
			SERCOM0->I2CM.INTFLAG.reg = flags;
			return;
		}
	}
};