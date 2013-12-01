/**
 * @file   WebInterface.cpp
 * @Author icke
 * @date   29.09.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include "WebInterface.h"

namespace icke2063 {
namespace MB_Gateway {

WebInterface::WebInterface(uint16_t port):m_port(port) {
	logger = &log4cpp::Category::getInstance(std::string("SummerySlave"));
	logger->setPriority(log4cpp::Priority::DEBUG);
	//if (console)logger->addAppender(console);

	logger->info("WebInterface@%i",m_port);

	p_server_thread.reset(new thread(&WebInterface::thread_function, this)); // create new scheduler thread

}

WebInterface::~WebInterface() {
	if(server.get()){
		server->shutdown();
		if(p_server_thread.get() && p_server_thread->joinable()){
			p_server_thread->join();
		}
	}
}

void WebInterface::thread_function (void){


	server.reset( new tnt::Tntnet());

	server->listen("0.0.0.0", m_port);

	//define rootfile
	server->mapUrl("^/$", "slave_list").setPathInfo("/slave_list");

	//map html files
	server->mapUrl("^/(.*).html", "$1");

	//map images
	server->mapUrl("^/images/(.*).png", "$1");
	server->mapUrl("^/images/(.*).jpg", "$1");
	server->mapUrl("^/images/(.*).gif", "$1");
	server->mapUrl("^/(.*).ico", "$1");

	//map stylesheets
	server->mapUrl("^/css/(.*).css", "$1");

	//map JavaScripts
	server->mapUrl("^/js/(.*).js", "$1");

	server->run();

}

} /* namespace MB_Gateway */
} /* namespace icke2063 */
