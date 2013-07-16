/**
 * @file   SummerySlave.h
 * @Author icke
 * @date   03.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef SUMMERYSLAVE_H_
#define SUMMERYSLAVE_H_

#include <VirtualRTUSlave.h>

//ext lib
#include <boost/thread.hpp>

//own lib
#include <Logger.h>

namespace MB_Gateway {

class SummerySlave: public MB_Gateway::VirtualRTUSlave, public Logger {
public:
	SummerySlave(uint8_t ID);
	virtual ~SummerySlave();
	virtual uint8_t getType( void ){return 0x01;}

private:
	bool init(void);

	boost::thread* p_scanner_thread;
	bool m_running;

	virtual void thread_function (void);

    boost::condition_variable m_Condition;  // Condition variable for timed_wait
    boost::mutex m_Mutex;                   // Mutex
    unsigned int m_timeout;
};

} /* namespace MB_Gateway */
#endif /* SUMMERYSLAVE_H_ */
