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

#include <handler/DefaultHandler.h>
using namespace icke2063::MB_Framework;

#include <Logger.h>
#include "modbus.h"

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

enum address_mode {
	_8bit = 0x01, _16bit = 0x02
};

class MultiByteHandler: public DefaultHandler {
public:
	MultiByteHandler(enum address_mode mode = _8bit, int16_t byte_count = -1) :
			m_mode(mode), m_byte_count(byte_count) {
		logger->info("MultiByteHandler");
	}
	virtual ~MultiByteHandler() {
	}

	virtual int handleReadAccess(MBHandlerParam *param);
	virtual int checkWriteAccess(MBHandlerParam *param);
	virtual int handleWriteAccess(MBHandlerParam *param);

private:

	enum address_mode m_mode;
	int16_t m_byte_count;
};

} /* namespace I2C */
}/* namespace MB_Gateway */
}/* namespace icke2063 */
#endif /* MULTIBYTEHANDLER_H_ */
