/**
 * @file   VirtualRTUSlave.h
 * @Author icke
 * @date   08.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef VIRTUALRTUSLAVE_H_
#define VIRTUALRTUSLAVE_H_

#include "stddef.h"
#include <MBVirtualRTUSlave.h>

#include <modbus-private.h>

namespace MB_Gateway {

class VirtualRTUSlave: public MB_Framework::MBVirtualRTUSlave {
public:
	VirtualRTUSlave(uint8_t SlaveAddr):MBVirtualRTUSlave(SlaveAddr),m_mapping(NULL) {}
	virtual ~VirtualRTUSlave() {}
	virtual uint8_t getType( void ){return 0x00;}
	modbus_mapping_t *getMappingDB(void){return m_mapping;}
protected:
	modbus_mapping_t *m_mapping;
};

} /* namespace MB_Gateway */
#endif /* VIRTUALRTUSLAVE_H_ */
