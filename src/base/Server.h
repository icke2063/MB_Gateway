/**
 * @file   Server.h
 * @Author icke2063
 * @date   26.05.2013
 * @brief  Brief description of file.
 *
 * Copyright © 2013 icke2063 <icke2063@gmail.com>
 *
 * This framework is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef SERVER_H_
#define SERVER_H_

//std libs
#include <stdint.h>
#include <list>

#include <build_options.h>

#ifndef ICKE2063_CRUMBY_NO_CPP11
	#include <memory>
	#include <thread>

	#define SERVER_H_NS std

#else
	#include <boost/thread.hpp>
	#include <boost/scoped_ptr.hpp>

	#define SERVER_H_NS boost
  #endif

//MB_Framework
#include <MBServer.h>
#include <ThreadPool.h>


namespace icke2063 {
namespace MB_Gateway {

class Server :
	public icke2063::MB_Framework::MBServer {
public:
	Server(uint16_t port, SERVER_H_NS::shared_ptr<icke2063::threadpool::ThreadPool> ext_pool);
	virtual ~Server();

private:
	/**
	 * server thread function
	 * - accept incoming connect requests
	 * - create new Connection objects
	 */
	void waitForConnection(void);

	/**
	 * Thread function to handle all connections.
	 * - one select for all open connections
	 * - remove finished connections
	 */
	void connection_handler(void);


	std::auto_ptr<SERVER_H_NS::thread> m_server_thread;
	std::auto_ptr<SERVER_H_NS::thread> m_conn_handler_thread;


	bool m_server_running;
	int m_server_socket;

	SERVER_H_NS::shared_ptr<icke2063::threadpool::ThreadPool> pool;

};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* SERVER_H_ */
