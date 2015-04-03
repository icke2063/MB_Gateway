/**
 * @file   I2CComm.h
 * @Author icke
 * @date   12.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef I2CCOMM_H_
#define I2CCOMM_H_

#include <build_options.h>

//std lib
#ifndef ICKE2063_CRUMBY_NO_CPP11
  #include <mutex>
#define I2CCOMM_H_NS std
#else
  #include <boost/thread/mutex.hpp>
#define I2CCOMM_H_NS boost
  
#endif


#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdint.h>
#include <string>

#define MAX_I2C_RETRY			0x10
#define MIN_I2C_SLAVE_ADR		0x00
#define MAX_I2C_SLAVE_ADR		0x77

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

class I2C_Comm {
public:
	I2C_Comm();
	I2C_Comm(std::string& i2c_master_path);
	virtual ~I2C_Comm();

	// Initialize functions

	/**
	 * open i2c bus with stored path
	 */
	bool i2cOpen(){return m_open_i2c_bus;}
	/**
	 * open i2c bus with given path
	 * - store given path
	 * - reopen i2cbus if different path as already stored
	 */
	bool i2cOpen(std::string& path); // open i2cbus at given path
	void i2cClose(); // close i2cbus
	bool i2cSetAddress(unsigned char address); // changes slave address

	// Communication functions
	bool Write_I2C_Bytes(unsigned char DEVICE_ADDR, uint8_t *databuffer,
			size_t address_size, size_t write_size);
	bool Read_I2C_Bytes(unsigned char DEVICE_ADDR, uint8_t *databuffer,
			size_t address_size, size_t read_size);

	int get_i2c_register(unsigned char addr, unsigned short reg,
			unsigned char *val);

	// special functions
	virtual bool *ScanBus(void);

private:
	int m_i2cFD; //file deskriptor for I2C connection
	std::string m_path;
	bool m_open_i2c_bus;

	void resetLivelist(void);
	bool livelist[MAX_I2C_SLAVE_ADR]; //list of found I2C slaves

	I2CCOMM_H_NS::mutex i2cbus_lock; //lock for functor list
};

} /* namespace I2C */
}/* namespace MB_Gateway */
}/* namespace icke2063 */
#endif /* I2CCOMM_H_ */
