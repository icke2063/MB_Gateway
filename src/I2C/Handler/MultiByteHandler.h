/**
 * @file   MultiByteHandler.h
 * @Author icke
 * @date   13.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef MULTIBYTEHANDLER_H_
#define MULTIBYTEHANDLER_H_

#include <build_options.h>

/** C++11 */
#include <memory>

#include <handler/DefaultHandler.h>
#include <I2CComm.h>

#include "modbus/modbus.h"

#include <i2c_logging_macros.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

enum address_mode {
	_8bit = 0x01, _16bit = 0x02
};

class MultiByteHandler:
	public DefaultHandler {
public:
	MultiByteHandler(std::shared_ptr<I2C_Comm> sp_i2c_comm
			, enum address_mode mode = _8bit
			, int16_t byte_count = -1) :
			m_mode(mode), m_byte_count(byte_count), m_sp_i2c_comm(sp_i2c_comm)
	{
		i2c_INFO_WRITE("MultiByteHandler");
	}
	virtual ~MultiByteHandler() {
	}

	virtual int handleReadAccess(icke2063::MB_Framework::MBHandlerParam *param);
	virtual int checkWriteAccess(icke2063::MB_Framework::MBHandlerParam *param);
	virtual int handleWriteAccess(icke2063::MB_Framework::MBHandlerParam *param);

protected:
	std::shared_ptr<I2C_Comm> m_sp_i2c_comm;
private:

	enum address_mode m_mode;
	int16_t m_byte_count;
};

} /* namespace I2C */
}/* namespace MB_Gateway */
}/* namespace icke2063 */
#endif /* MULTIBYTEHANDLER_H_ */
