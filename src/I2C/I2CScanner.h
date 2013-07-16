/**
 * @file   I2CScanner.h
 * @Author icke
 * @date   02.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef I2CSCANNER_H_
#define I2CSCANNER_H_

#define MAX_SCAN_FAIL 5

//ext lib
#include <boost/thread.hpp>

//own lib
#include <Logger.h>

#define DEFAULT_SCAN_TIMEOUT_MS	10000

namespace MB_Gateway {
namespace I2C {

class I2C_Scanner: public Logger {
public:
	I2C_Scanner(unsigned int timeout = DEFAULT_SCAN_TIMEOUT_MS);
	virtual ~I2C_Scanner();

private:

	boost::thread* p_scanner_thread;
	bool m_running;

	virtual void thread_function (void);

    boost::condition_variable m_Condition;  // Condition variable for timed_wait
    boost::mutex m_Mutex;                   // Mutex
    unsigned int m_timeout;

};

}} /* namespace MB_Gateway::I2C */
#endif /* I2CSCANNER_H_ */
