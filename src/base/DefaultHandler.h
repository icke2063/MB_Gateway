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
using namespace MB_Framework;

namespace MB_Gateway {

class DefaultHandler:public MBHandlerInt, public MB_Gateway::Logger {
public:
	DefaultHandler();
	virtual ~DefaultHandler(){};
};

} /* namespace MB_Gateway */
#endif /* DEFAULTHANDLER_H_ */
