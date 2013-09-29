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

using namespace icke2063::MB_Framework;

namespace icke2063 {
namespace MB_Gateway {

class VirtualRTUSlave: public MBVirtualRTUSlave {
public:
	VirtualRTUSlave(uint8_t SlaveAddr):MBVirtualRTUSlave(SlaveAddr),m_mapping(NULL) {}
	virtual ~VirtualRTUSlave() {
		if(m_mapping)modbus_mapping_free(m_mapping);
	}
	virtual uint8_t getType( void ){return 0x00;}
	modbus_mapping_t *getMappingDB(void){return m_mapping;}

	virtual bool init(void) = 0;

protected:
	modbus_mapping_t *m_mapping;
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */

#endif /* VIRTUALRTUSLAVE_H_ */
