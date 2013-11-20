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

#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>
using namespace std;

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

	unique_ptr<std::thread> m_scanner_thread;
	bool m_running;

	virtual void thread_function (void);

    std::condition_variable m_Condition;  // Condition variable for timed_wait
    std::mutex m_Mutex;                   // Mutex
    unsigned int m_timeout;

    I2C_Slave *findSlaveType (uint8_t slaveaddress);
};

}}} /* namespace icke2063::MB_Gateway::I2C */
#endif /* I2CSCANNER_H_ */
