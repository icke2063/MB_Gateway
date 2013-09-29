/**
 * @file   I2CComm.cpp
 * @Author icke
 * @date   12.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include "I2C-Comm/I2CComm.h"

#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

I2C_Comm::I2C_Comm() :
		m_i2cFD(-1), m_path("") {
	resetLivelist();

}

I2C_Comm::~I2C_Comm() {
	i2cClose();
}

bool I2C_Comm::i2cOpen(std::string path) {
	if (path.compare(m_path) != 0) {
		if (m_i2cFD > 0)
			i2cClose(); //close existing connection

		m_path = path;
		if (((m_i2cFD = open(m_path.c_str(), O_RDWR))) < 0)
			return false; //open new connection
	}
	return true;
}

void I2C_Comm::i2cClose() {
	close(m_i2cFD);
	m_i2cFD = -1;
	m_path = "";
}

bool I2C_Comm::i2cSetAddress(unsigned char address) {
	unsigned char retry = MAX_I2C_RETRY;
	if (m_i2cFD > 0) {
		boost::lock_guard<boost::mutex> lock(*(i2cbus_lock.getMutex()));
		while (retry-- != 0 && ioctl(m_i2cFD, I2C_SLAVE, address) < 0) {
		}
		if (retry)
			return true;
	}
	return false;
}

bool I2C_Comm::Write_I2C_Bytes(unsigned char DEVICE_ADDR, uint8_t *databuffer,
		size_t address_size, size_t write_size) {
	size_t n = address_size + write_size;

	unsigned char inbuf;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[1];

	messages[0].addr = DEVICE_ADDR;
	messages[0].flags = 0;
	messages[0].len = n;
	messages[0].buf = databuffer;

	/* Send the request to the kernel and get the result back */
	packets.msgs = messages;
	packets.nmsgs = 1;

	{
		boost::lock_guard<boost::mutex> lock(*(i2cbus_lock.getMutex()));
		if (ioctl(m_i2cFD, I2C_RDWR, &packets) < 0) {
			perror("Unable to send data");
			return false;
		}
	}
	return true;
}

bool I2C_Comm::Read_I2C_Bytes(unsigned char DEVICE_ADDR, uint8_t *databuffer,
		size_t address_size, size_t read_size) {
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[2];

	/*
	 * In order to read a register, we first do a "dummy write" by writing
	 * 0 bytes to the register we want to read from.  This is similar to
	 * the packet in set_i2c_register, except it's 1 byte rather than 2.
	 */

	messages[0].addr = DEVICE_ADDR;
	messages[0].flags = 0;
	messages[0].len = address_size;
	messages[0].buf = databuffer;

	/* The data will get returned in this structure */
	messages[1].addr = DEVICE_ADDR;
	messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
	messages[1].len = read_size;
	messages[1].buf = databuffer;

	/* Send the request to the kernel and get the result back */
	packets.msgs = messages;
	packets.nmsgs = 2;
	{
		boost::lock_guard<boost::mutex> lock(*(i2cbus_lock.getMutex()));
		if (ioctl(m_i2cFD, I2C_RDWR, &packets) < 0) {
			perror("Unable to send data");
			return false;
		}

	}
	return true;
}

bool *I2C_Comm::ScanBus(void) {
	int slave_adr = 0;
	unsigned char retry;
	uint8_t c;
	if (m_i2cFD > 0) {
		// loop over whole range
		for (slave_adr = MIN_I2C_SLAVE_ADR; slave_adr <= MAX_I2C_SLAVE_ADR;
				slave_adr++) {
			retry = MAX_I2C_RETRY;
			if (!i2cSetAddress(slave_adr))
				return NULL;

			if (read(m_i2cFD, &c, 1) != 1) {
				livelist[slave_adr] = false;
			} else {
				livelist[slave_adr] = true;
			}
		}
		return livelist;
	}
	return NULL;
}

void I2C_Comm::resetLivelist(void) {
	int i;
	for (i = MIN_I2C_SLAVE_ADR; i <= MAX_I2C_SLAVE_ADR; i++) {
		livelist[i] = false;
	}
}

} /* namespace I2C */
}/* namespace MB_Gateway */
}/* namespace icke2063 */

