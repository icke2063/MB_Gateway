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

#include <build_options.h>

//std lib
#ifndef ICKE2063_CRUMBY_NO_CPP11
  #include <memory>
  #include <thread>
#define WEBINTERFACE_H_NS std
#else
  #include <boost/shared_ptr.hpp>
  #include <boost/thread.hpp>
#define WEBINTERFACE_H_NS boost
#endif


#include <tnt/tntnet.h>

//own lib
#include <Logger.h>

namespace icke2063 {
namespace MB_Gateway {

class WebInterface : public common_cpp::Logger{
public:
	WebInterface(uint16_t port=8000);
	virtual ~WebInterface();

private:
	uint16_t m_port;

	std::auto_ptr<tnt::Tntnet> server;
	std::auto_ptr<WEBINTERFACE_H_NS::thread> p_server_thread;

	bool m_running;

	virtual void thread_function (void);
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* WEBINTERFACE_H_ */
