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

#include <build_options.h>

//std lib
#ifndef ICKE2063_CRUMBY_NO_CPP11
  #include <memory>
  #include <thread>
  #include <mutex>
#include <condition_variable>

#define I2CSCANNER_H_NS std
#else
	#include <boost/thread.hpp>
	#include <boost/shared_ptr.hpp>
	#include <boost/scoped_ptr.hpp>
	#include <boost/thread/condition_variable.hpp>

#define I2CSCANNER_H_NS boost
#endif

#define MAX_SCAN_FAIL 5

//own lib
#include <I2CSlave.h>

#define DEFAULT_SCAN_TIMEOUT_MS	1000

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

class I2C_Scanner
{
public:
	I2C_Scanner(unsigned int timeout = DEFAULT_SCAN_TIMEOUT_MS);
	virtual ~I2C_Scanner();

private:


#ifndef ICKE2063_CRUMBY_NO_CPP11
	std::unique_ptr<std::thread> m_scanner_thread;
#else
	boost::scoped_ptr<boost::thread> m_scanner_thread;
#endif

	bool m_running;

	virtual void thread_function (void);

	I2CSCANNER_H_NS::condition_variable m_Condition;  // Condition variable for timed_wait
	I2CSCANNER_H_NS::mutex m_Mutex;                   // Mutex
    unsigned int m_timeout;

    I2CSCANNER_H_NS::shared_ptr<I2C_Slave> findSlaveType (uint8_t slaveaddress);
};

}}} /* namespace icke2063::MB_Gateway::I2C */
#endif /* I2CSCANNER_H_ */
