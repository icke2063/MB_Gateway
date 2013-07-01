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

namespace MB_Gateway {
namespace I2C {

class HolRegHandler: public MB_Gateway::I2C::SingleRegisterHandler {
public:
	HolRegHandler(enum address_mode mode = _8bit) :
			SingleRegisterHandler(mode) {
		logger->info("HolRegHandler");
		enableReadInpReg = false;
	}
	virtual ~HolRegHandler() {
	}
};

} /* namespace I2C */
} /* namespace MB_Gateway */
#endif /* HOLREGHANDLER_H_ */
