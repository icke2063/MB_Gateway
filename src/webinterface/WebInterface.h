/**
 * @file   WebInterface.h
 * @Author icke
 * @date   29.09.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef WEBINTERFACE_H_
#define WEBINTERFACE_H_

//ext lib
#include <memory>
#include <thread>
using namespace std;

#include <tnt/tntnet.h>

//own lib
#include <Logger.h>

using namespace icke2063::common_cpp;

namespace icke2063 {
namespace MB_Gateway {

class WebInterface : public Logger{
public:
	WebInterface(uint16_t port=8000);
	virtual ~WebInterface();

private:
	uint16_t m_port;
	unique_ptr<tnt::Tntnet> server;

	unique_ptr<std::thread> p_server_thread;
	bool m_running;

	virtual void thread_function (void);
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* WEBINTERFACE_H_ */
