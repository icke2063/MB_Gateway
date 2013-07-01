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
using namespace std;

//MB_Framework
#include <MBServer.h>
using namespace MB_Framework;

#include <ThreadPool.h>
#include <Logger.h>

namespace MB_Gateway {

class Server : public MBServer, public Logger{
public:
	Server(ThreadPool *pool, uint16_t port);
	virtual ~Server();

private:
	/**
	 * server worker function for TreadPool
	 */
	void functor_function(void);

	bool m_server_running;

//	ThreadPool *p_pool;

	int m_server_socket;
};

} /* namespace MB_Gateway */
#endif /* SERVER_H_ */
