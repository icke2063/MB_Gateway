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

using namespace icke2063::common_cpp;

namespace icke2063 {
namespace MB_Gateway {

class WebInterface : public Logger{
public:
	WebInterface(uint16_t port=8000);
	virtual ~WebInterface();

private:
	uint16_t m_port;
#ifndef ICKE2063_CRUMBY_NO_CPP11
	std::unique_ptr<tnt::Tntnet> server;
	std::unique_ptr<std::thread> p_server_thread;
#else
	boost::scoped_ptr<tnt::Tntnet> server;
	boost::scoped_ptr<boost::thread> p_server_thread;
#endif
	bool m_running;

	virtual void thread_function (void);
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* WEBINTERFACE_H_ */
