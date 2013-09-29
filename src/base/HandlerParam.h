/**
 * @file   HandlerParam.h
 * @Author icke
 * @date   02.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef HANDLERPARAM_H_
#define HANDLERPARAM_H_

//libmodbus
#include <modbus-private.h>

#include <MBHandlerInt.h>

using namespace icke2063::MB_Framework;

namespace icke2063 {
namespace MB_Gateway {

class HandlerParam: public MB_Framework::MBHandlerParam {
public:
	HandlerParam(uint8_t slave, uint8_t function, uint16_t address, uint16_t count, modbus_mapping_t *mb_mapping):
	m_slave(slave),
	m_function(function),
	m_address(address),
	m_count(count),p_mb_mapping(mb_mapping){};
	virtual ~HandlerParam(){};

	const uint8_t m_slave;
	const uint8_t m_function;
	const uint16_t m_address;
	const uint16_t m_count;
	const modbus_mapping_t *p_mb_mapping;
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* HANDLERPARAM_H_ */
