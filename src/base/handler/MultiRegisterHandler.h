/**
 * @file   MultiRegisterHandler.h
 * @Author icke
 * @date   03.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef MULTIREGISTERHANDLER_H_
#define MULTIREGISTERHANDLER_H_

#include "DefaultHandler.h"

#include "modbus/modbus.h"
#include <modbus_logging_macros.h>

namespace icke2063 {
namespace MB_Gateway {

class MultiRegisterHandler: public MB_Gateway::DefaultHandler {
public:
	MultiRegisterHandler(modbus_mapping_t * pmap = NULL, int16_t count = -1) :
			p_mb_mapping(pmap), m_register_count(count) {
		modbus_INFO_WRITE("MultiRegisterHandler");
	}
	virtual ~MultiRegisterHandler(){}

	virtual int handleReadAccess(icke2063::MB_Framework::MBHandlerParam *param);
	virtual int checkWriteAccess(icke2063::MB_Framework::MBHandlerParam *param);
	virtual int handleWriteAccess(icke2063::MB_Framework::MBHandlerParam *param);

private:
	modbus_mapping_t *p_mb_mapping;
	int16_t m_register_count;
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */

#endif /* MULTIREGISTERHANDLER_H_ */
