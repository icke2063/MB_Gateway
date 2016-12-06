/**
 * @file   SingleRegisterHandler.h
 * @Author icke
 * @date   13.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef SINGLEREGISTERHANDLER_H_
#define SINGLEREGISTERHANDLER_H_

#include <MultiByteHandler.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

class SingleRegisterHandler: public MultiByteHandler {
public:
	SingleRegisterHandler(std::shared_ptr<I2C_Comm> sp_i2c_comm, enum address_mode mode = _8bit) :
			MultiByteHandler(sp_i2c_comm, mode, 2) {
		i2c_INFO_WRITE("SingleRegisterHandler");
	}
	virtual ~SingleRegisterHandler() {
	}
};

} /* namespace I2C */
}/* namespace MB_Gateway */
}/* namespace icke2063 */
#endif /* SINGLEREGISTERHANDLER_H_ */
