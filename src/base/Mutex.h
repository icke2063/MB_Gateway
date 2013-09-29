/**
 * @file   Mutex.h
 * @Author icke
 * @date   19.09.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include <boost/thread.hpp>
#include <MBMutex.h>

using namespace icke2063::MB_Framework;

namespace icke2063 {
namespace MB_Gateway {

class Mutex: public MB_Framework::MBMutex {
public:
	Mutex(){};
	virtual ~Mutex(){};

	virtual boost::mutex *getMutex(void){return &m_mutex;}

private:
	boost::mutex m_mutex;
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* MUTEX_H_ */
