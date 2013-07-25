/**
 * @file   HandlerList.h
 * @Author icke
 * @date   07.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef HANDLERLIST_H_
#define HANDLERLIST_H_

#include <Mutex.h>
#include <MBHandlerList.h>

namespace MB_Gateway {

class HandlerList:public MB_Framework::MBHandlerList {
public:
	HandlerList() {
		p_handlerlist_lock = new Mutex;

	}
	virtual ~HandlerList() {}
};

} /* namespace MB_Gateway */
#endif /* HANDLERLIST_H_ */
