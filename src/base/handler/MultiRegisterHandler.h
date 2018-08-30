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

/* MB Framework */
#include <MBHandlerInt.h>

#include "modbus/modbus.h"
#include <modbus_logging_macros.h>

namespace icke2063 {
namespace MB_Gateway {

class MultiRegisterHandler:
		public icke2063::MB_Framework::MBHandlerInt
{
public:
	/**
	 * Default constructor
	 *
	 * @param pmap: 	reference to associated storage
	 * @param count:	max. register count handled on single handler call
	 */
	MultiRegisterHandler(modbus_mapping_t * pmap = NULL, int16_t count = -1) :
			p_mb_mapping(pmap), m_register_count(count)
	{
		modbus_INFO_WRITE("MultiRegisterHandler");
	}

	/**
	 * Default destructor
	 */
	virtual ~MultiRegisterHandler(){}

	/**
	 * handle read request
	 *
	 * - copy data from internal storage to parameter
	 *
	 * @param param[in/out]:	reference to parameter object
	 * @return count of handled mb register
	 */
	virtual int handleReadAccess(icke2063::MB_Framework::MBHandlerParam *param);

	/**
	 * check write request accessibility
	 *
	 * - only check if the write request will success
	 *
	 * @param param[in]:	reference to parameter object
	 * @return coutn of handled mb register
	 */
	virtual int checkWriteAccess(icke2063::MB_Framework::MBHandlerParam *param);
	virtual int handleWriteAccess(icke2063::MB_Framework::MBHandlerParam *param);

private:
	modbus_mapping_t *p_mb_mapping;
	int16_t m_register_count;
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */

#endif /* MULTIREGISTERHANDLER_H_ */
