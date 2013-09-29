/**
 * @file   DefaultHandler.cpp
 * @Author icke
 * @date   14.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include "DefaultHandler.h"

namespace icke2063 {
namespace MB_Gateway {

DefaultHandler::DefaultHandler() {
	logger = &log4cpp::Category::getInstance(std::string("DefaultHandler"));
	logger->setPriority(log4cpp::Priority::DEBUG);
	//if (console)logger->addAppender(console);
}
} /* namespace MB_Gateway */
} /* namespace icke2063 */
