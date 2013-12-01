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

#include <MBHandlerList.h>

using namespace icke2063::MB_Framework;
using namespace icke2063::common_cpp;

namespace icke2063 {
namespace MB_Gateway {

class HandlerList:public MBHandlerList {
public:
	HandlerList() {
		m_handlerlist_lock.reset(new mutex());

	}
	virtual ~HandlerList() {}
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* HANDLERLIST_H_ */
