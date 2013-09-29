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
	SingleRegisterHandler(enum address_mode mode = _8bit) :
			MultiByteHandler(mode, 2) {
		logger->info("SingleRegisterHandler");
	}
	virtual ~SingleRegisterHandler() {
	}
};

} /* namespace I2C */
}/* namespace MB_Gateway */
}/* namespace icke2063 */
#endif /* SINGLEREGISTERHANDLER_H_ */
