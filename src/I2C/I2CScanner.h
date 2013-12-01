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

//std lib
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
  #include <memory>
  #include <thread>
  #include <condition_variable>
  #include <mutex>
  using namespace std;
#else
  #include <boost/thread.hpp>

  using namespace boost;
#endif

//own lib
#include <Logger.h>
using namespace icke2063::common_cpp;

#include <I2CSlave.h>

#define DEFAULT_SCAN_TIMEOUT_MS	1000

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

class I2C_Scanner: public Logger {
public:
	I2C_Scanner(unsigned int timeout = DEFAULT_SCAN_TIMEOUT_MS);
	virtual ~I2C_Scanner();

private:

	unique_ptr<thread> m_scanner_thread;
	bool m_running;

	virtual void thread_function (void);

    condition_variable m_Condition;  // Condition variable for timed_wait
    mutex m_Mutex;                   // Mutex
    unsigned int m_timeout;

    shared_ptr<I2C_Slave> findSlaveType (uint8_t slaveaddress);
};

}}} /* namespace icke2063::MB_Gateway::I2C */
#endif /* I2CSCANNER_H_ */
