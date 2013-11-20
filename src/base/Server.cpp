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
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <thread>
using namespace std;

#include <sys/select.h>
#include <boost/concept_check.hpp>


// libmodbus
#include "modbus.h"
#include "modbus-private.h"

#include "Connection.h"
#include "Mutex.h"

#include <DummyFunctor.h>

namespace icke2063 {
namespace MB_Gateway {

Server::Server(uint16_t port):
		MBServer(port),
		m_server_running(true),m_server_socket(-1){
	/*
	 * - set category name
	 * - connect with console
	 * - set loglevel
	 * @todo do this by configfile
	 */
	logger = &log4cpp::Category::getInstance(std::string("Server"));
	logger->setPriority(log4cpp::Priority::DEBUG);
	if(console)logger->addAppender(console);

	logger->info("Modbus TCP Server@%i",m_port);

	m_conn_lock.reset(new MB_Gateway::Mutex);

	pool.reset(new DelayedThreadPool());	//get new ThreadPool
	pool->setHighWatermark(5);

	shared_ptr<Dummy_Functor> dummy = shared_ptr<Dummy_Functor>(new Dummy_Functor());
	
	
	//Clock::time_point deadline = Clock::to_time_t(Clock::now() + std::chrono::seconds(5));
	struct timeval now;
	gettimeofday(&now,0);
	now.tv_sec += 5;
	
	
	pool->addDelayedFunctor(dummy,now);
	pool->addDelayedFunctor(dummy,now);
	now.tv_sec += 5;
	
	pool->addDelayedFunctor(dummy,now);
	pool->addDelayedFunctor(dummy,now);
	now.tv_sec += 5;
	
	pool->addDelayedFunctor(dummy,now);
	now.tv_sec += 5;
	
	pool->addDelayedFunctor(dummy,now);
	
	
	m_server_thread.reset(new std::thread(&Server::waitForConnection, this));	// create new scheduler thread
	m_conn_handler_thread.reset(new std::thread(&Server::connection_handler, this));
}

Server::~Server() {
	logger->info("enter ~Server");
	m_server_running = false;
	if(m_server_thread.get() && m_server_thread->joinable())m_server_thread->join();
	logger->info("leave ~Server");
}

void Server::waitForConnection(void){
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
		return;
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

			if(m_conn_lock.get() != NULL){
				std::lock_guard<std::mutex> lock(*((MB_Gateway::Mutex*)m_conn_lock.get())->getMutex().get());
				openConnections.push_back(shared_ptr<Connection>(new Connection(ctx_tmp)));
			}
			modbus_free(ctx_tmp);
		}else
		{
			logger->debug("error modbus_tcp_accept");
		}

	}
	modbus_close(ctx);
	modbus_free(ctx);

}

void Server::connection_handler (void){
	/* select */
    fd_set rfds;
    int maxFD=0;
    struct timeval tv;
    int retval;
    list<shared_ptr<MBConnection>>::iterator conn_it;
    Connection *curConn = NULL;


    while (m_server_running) {

		/* Wait up to 500 useconds. */
		tv.tv_sec = 0;
		tv.tv_usec = 500;

		FD_ZERO(&rfds);
		maxFD = 0;


		if(m_conn_lock.get() != NULL){
			std::lock_guard<std::mutex> lock(*((MB_Gateway::Mutex*)m_conn_lock.get())->getMutex().get());

			conn_it = openConnections.begin();

			while(conn_it != openConnections.end()){//loop over all connections
				shared_ptr<MBConnection> tmpConn = *conn_it;
				curConn =  dynamic_cast<MB_Gateway::Connection *> (tmpConn.get());

				if(curConn && curConn->getStatus() == MBConnection::open){//add open connection into set
					FD_SET(curConn->getConnInfo()->s, &rfds);
					if(maxFD < curConn->getConnInfo()->s)maxFD = curConn->getConnInfo()->s;
				}

				if(curConn->getStatus() == MBConnection::closed){//remove closed connection
					logger->debug("remove Connection\n");
					//delete curConn;
					conn_it = openConnections.erase(conn_it);
					continue;
				}
				++conn_it;
			}


			try {
			   retval = select(maxFD+1, &rfds, &rfds, NULL, &tv);
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


			conn_it = openConnections.begin();

			while(conn_it != openConnections.end()){//loop over all connections
				shared_ptr<MBConnection> tmpConn = *conn_it;
				curConn =  dynamic_cast<MB_Gateway::Connection *> (tmpConn.get());

				if(curConn && FD_ISSET(curConn->getConnInfo()->s, &rfds)){//add open connection into set
					curConn -> setStatus(MBConnection::busy);
					//use threadpool
					pool->addFunctor(curConn->getFunctor());
				}
				++conn_it;
			}
		}
    }
}

} /* namespace MB_Gateway */
} /* namespace icke2063 */
