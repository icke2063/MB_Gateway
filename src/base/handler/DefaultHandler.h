/**
 * @file   DefaultHandler.h
 * @Author icke
 * @date   14.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef DEFAULTHANDLER_H_
#define DEFAULTHANDLER_H_

#include <MBHandlerInt.h>
#include <Logger.h>

namespace icke2063 {
namespace MB_Gateway {

class DefaultHandler:
	public icke2063::MB_Framework::MBHandlerInt,
	public icke2063::common_cpp::Logger {
public:
	DefaultHandler();
	virtual ~DefaultHandler(){};
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* DEFAULTHANDLER_H_ */
