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

#include <MBMutex.h>

#include <mutex>
#include <memory>
using namespace std;

using namespace icke2063::MB_Framework;

namespace icke2063 {
namespace MB_Gateway {

class Mutex: public MB_Framework::MBMutex {
public:
	Mutex(){m_mutex = shared_ptr<std::mutex>(new std::mutex());}
	virtual ~Mutex(){};

	virtual shared_ptr<std::mutex> getMutex(void){return m_mutex;}

private:
	shared_ptr<std::mutex> m_mutex;
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* MUTEX_H_ */
