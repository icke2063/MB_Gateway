/**
 * @file   HolRegHandlerRO.h
 * @Author icke
 * @date   15.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef HOLREGHANDLERRO_H_
#define HOLREGHANDLERRO_H_

#include <MultiByteHandler.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

class HolRegHandlerRO: public MultiByteHandler {
public:
	HolRegHandlerRO(std::shared_ptr<I2C_Comm> sp_i2c_comm, enum address_mode mode = _8bit) :
		MultiByteHandler(sp_i2c_comm, mode) {
		i2c_INFO_WRITE("HolRegHandlerRO");
		enableReadInpReg = false;
	} //disable input register support
	virtual ~HolRegHandlerRO() {
	}

	virtual int checkWriteAccess(MB_Framework::MBHandlerParam *param) {
		return 0;
	} //disable write support
	virtual int handleWriteAccess(MB_Framework::MBHandlerParam *param) {
		return 0;
	} //disable write support
};

} /* namespace I2C */
} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* HOLREGHANDLERRO_H_ */
