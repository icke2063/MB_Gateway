/**
 * @file   Server.cpp
 * @Author icke2063
 * @date   26.05.2013
 * @brief  MBServer implementation
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

#include "Server.h"

#include <iostream>
using namespace std;

#include <sys/select.h>

// libmodbus
#include "modbus.h"
#include "modbus-private.h"

#include "Connection.h"

namespace MB_Gateway {

Server::Server(ThreadPool *pool, uint16_t port):
		m_server_running(true),m_server_socket(-1),p_pool(pool),m_port(port){

	/**
	 * Init Logging
	 * - set category name
	 * - connect with console
	 * - set loglevel
	 * @todo do this by configfile
	 */
	logger = &log4cpp::Category::getInstance(std::string("Server"));
	logger->setPriority(log4cpp::Priority::DEBUG);
	if(console)logger->addAppender(console);

	logger->info("Modbus TCP Server@%i",m_port);
}

Server::~Server() {
	m_server_running = false;
	logger->info("~Server");
}

void Server::functor_function(void){
	/* select */
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* libmodbus */
	modbus_t *ctx;

	logger->debug("Server Functor@%i",m_port);

	//prepare listening
	ctx = modbus_new_tcp("127.0.0.1", m_port);
	if(ctx == NULL){
		logger->fatal("error: modbus_new_tcp");
		exit(0);
	}

	/* modbus_set_debug(ctx, TRUE); */
	//get new socket
	m_server_socket = modbus_tcp_listen(ctx, 1);
	if(m_server_socket == -1){
		logger->fatal("error: modbus_tcp_listen");
		exit(0);
	}

	logger->debug("Server FD:%i",m_server_socket);

	while (m_server_running) {

		/* Wait up to 500 useconds. */
		tv.tv_sec = 0;
		tv.tv_usec = 500;

		FD_ZERO(&rfds);
		FD_SET(m_server_socket, &rfds);

		try {
		   retval = select(m_server_socket+1, &rfds, &rfds, NULL, &tv);
		} catch (const std::exception& e) {;
			logger->error("error: select exception");
			continue;
		}

		if(retval == 0 ){
			continue;
		}

		if(retval < 0 ){
			logger->error("select error:%d - %s",retval,strerror(errno));
			continue;
		}



		//temporary fake connection struct (for multi client usage)
		modbus_t *ctx_tmp;
		ctx_tmp = modbus_new_tcp("127.0.0.1", m_port);

		cout << "before modbus accept" << endl;
		if (modbus_tcp_accept(ctx_tmp, &m_server_socket) != -1) {

			logger->debug("modbus_tcp_accept:%i",ctx_tmp->s);
			{
				if(p_pool)p_pool->addFunctor(new Connection(ctx_tmp));
				modbus_free(ctx_tmp);
			}
		}else
		{
			logger->debug("error modbus_tcp_accept");
		}

	}
	modbus_close(ctx);
	modbus_free(ctx);

}

} /* namespace MB_Gateway */
