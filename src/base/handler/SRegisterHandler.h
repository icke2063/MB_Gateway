/**
 * @file   SRegisterHandler.h
 * @Author icke
 * @date   04.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef SREGISTERHANDLER_H_
#define SREGISTERHANDLER_H_

#include <handler/MultiRegisterHandler.h>

namespace MB_Gateway {

class SRegisterHandler: public MultiRegisterHandler {
public:
	SRegisterHandler(modbus_mapping_t * pmap = NULL):MultiRegisterHandler(pmap,1){}
	virtual ~SRegisterHandler(){}
};

} /* namespace MB_Gateway */
#endif /* SREGISTERHANDLER_H_ */
