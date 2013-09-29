/**
 * @file   HolRegHandler.h
 * @Author icke
 * @date   15.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef HOLREGHANDLER_H_
#define HOLREGHANDLER_H_

#include <SingleRegisterHandler.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

class HolRegHandler: public MultiByteHandler {
public:
	HolRegHandler(enum address_mode mode = _8bit) :
		MultiByteHandler(mode) {
		logger->info("HolRegHandler");
		enableReadInpReg = false;
	}
	virtual ~HolRegHandler() {
	}
};

} /* namespace I2C */
} /* namespace MB_Gateway */
} /* namespace icke2063*/
#endif /* HOLREGHANDLER_H_ */
