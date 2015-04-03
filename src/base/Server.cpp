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

#ifndef ICKE2063_CRUMBY_NO_CPP11
  #include <mutex>
  using namespace std;
#else
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
  using namespace boost;
#endif


#include <sys/select.h>
#include <boost/concept_check.hpp>


// libmodbus
#include "modbus/modbus.h"


#include "Connection.h"
#include <modbus_logging_macros.h>

namespace icke2063 {
namespace MB_Gateway {

Server::Server(uint16_t port, shared_ptr<threadpool::ThreadPool> ext_pool):
		MBServer(port),
		m_server_running(true),m_server_socket(-1){

	modbus_NOTICE_WRITE("Modbus TCP Server@%i",m_port);

	m_conn_lock.reset(new mutex());

	if(ext_pool.get()){
	  pool = ext_pool;
	  modbus_INFO_WRITE("Server use extern ThreadPool");
	} else {
	  modbus_INFO_WRITE("Server use intern ThreadPool");
	  pool = shared_ptr<threadpool::ThreadPool>(new threadpool::ThreadPool());
	  pool->setHighWatermark(5);
	}
	

	try {
		m_server_thread.reset(new thread(&Server::waitForConnection, this));	// create new scheduler thread
		m_conn_handler_thread.reset(new thread(&Server::connection_handler, this));
	} catch (std::runtime_error& e) {
		modbus_CRIT_WRITE("Cannot create Server/Connection threads: %s\n",e.what());
		exit(0);
	}
}

Server::~Server() {
	modbus_NOTICE_WRITE("~Server");
	m_server_running = false;
	if(m_server_thread.get() && m_server_thread->joinable())m_server_thread->join();
	if(m_conn_handler_thread.get() && m_conn_handler_thread->joinable())m_conn_handler_thread->join();

	modbus_DEBUG_WRITE("leave ~Server");
}

void Server::waitForConnection(void){
	/* select */
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* libmodbus */
	modbus_t *ctx;

	modbus_DEBUG_WRITE("Server Thread@%i",m_port);

	//prepare listening
	ctx = modbus_new_tcp("127.0.0.1", m_port);
	if(ctx == NULL){
		modbus_CRIT_WRITE("error: modbus_new_tcp");
		return;
	}

	/* modbus_set_debug(ctx, TRUE); */
	//get new socket
	m_server_socket = modbus_tcp_listen(ctx, 1);
	if(m_server_socket == -1){
		modbus_CRIT_WRITE("error: modbus_tcp_listen");
		return;
	}

	modbus_DEBUG_WRITE("Server FD:%i",m_server_socket);

	while (m_server_running)
	{
		/* Wait up to 500 useconds. */
		tv.tv_sec = 0;
		tv.tv_usec = 500;

		FD_ZERO(&rfds);
		FD_SET(m_server_socket, &rfds);

		try {
		   retval = select(m_server_socket+1, &rfds, &rfds, NULL, &tv);
		} catch (const std::exception& e) {;
			modbus_ERROR_WRITE("error: select exception");
			continue;
		}

		if(retval == 0 ){
			continue;
		}

		if(retval < 0 ){
			modbus_ERROR_WRITE("select error:%d - %s",retval,strerror(errno));
			continue;
		}

		//temporary fake connection struct (for multi client usage)
		modbus_t *ctx_tmp;
		ctx_tmp = modbus_new_tcp("127.0.0.1", m_port);

		if (modbus_tcp_accept(ctx_tmp, &m_server_socket) != -1) {

			modbus_INFO_WRITE("modbus_tcp_accept:%i",modbus_get_socket(ctx_tmp));

			if(m_conn_lock.get() != NULL){
				lock_guard<mutex> lock(*m_conn_lock.get());

				try{
					shared_ptr<Connection> tmp(new Connection(ctx_tmp));
					openConnections.push_back(tmp);
				}catch(std::runtime_error& e){
					modbus_ERROR_WRITE("Cannot create Connection:%s", e.what());
				}
			}
		}else
		{
			modbus_ERROR_WRITE("error modbus_tcp_accept");
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
    std::list<shared_ptr<MB_Framework::MBConnection> >::iterator conn_it;

    while (m_server_running)
    {
		/* Wait up to 500 useconds. */
		tv.tv_sec = 0;
		tv.tv_usec = 500;

		FD_ZERO(&rfds);
		maxFD = 0;


		if(m_conn_lock.get() != NULL){
			lock_guard<mutex> lock(*m_conn_lock.get());

			conn_it = openConnections.begin();

			while(conn_it != openConnections.end()){//loop over all connections
				shared_ptr<Connection> curConn =  dynamic_pointer_cast<Connection> (*conn_it);

				if(curConn.get() && curConn->getStatus() == MB_Framework::MBConnection::open){//add open connection into set
					FD_SET(modbus_get_socket(curConn->getConnInfo()), &rfds);
					if(maxFD < modbus_get_socket(curConn->getConnInfo()))maxFD = modbus_get_socket(curConn->getConnInfo());
					curConn->setStatus(MB_Framework::MBConnection::wait);	//mark connection as in fdset
				}

				if(curConn->getStatus() == MB_Framework::MBConnection::closed){//remove closed connection
					modbus_DEBUG_WRITE("remove Connection[%d]\n",modbus_get_socket(curConn->getConnInfo()));
					//delete curConn; //done by shared_ptr
					conn_it = openConnections.erase(conn_it);
					continue;
				}
				++conn_it;
			}


			try {
			   retval = select(maxFD+1, &rfds, &rfds, NULL, &tv);
			} catch (const std::exception& e) {;
				modbus_ERROR_WRITE("error: select exception");
				continue;
			}

			if(retval == 0 ){
				continue;
			}

			if(retval < 0 ){
				modbus_ERROR_WRITE("select error:%d - %s",retval,strerror(errno));
				continue;
			}


			modbus_DEBUG_WRITE("handle connecton: %d\n",openConnections.size());

			conn_it = openConnections.begin();

			while(conn_it != openConnections.end()){//loop over all connections
				shared_ptr<Connection> curConn =  dynamic_pointer_cast<Connection>(*conn_it);
				if(curConn.get() && FD_ISSET(modbus_get_socket( curConn->getConnInfo() ), &rfds)){//handle connections with incoming data
					curConn -> setStatus(MB_Framework::MBConnection::busy);	//mark current connection as busy (handle current request)
					modbus_DEBUG_WRITE("data on socket[%d]\n",modbus_get_socket( curConn->getConnInfo()));
					//use threadpool
					threadpool::FunctorInt *tmpFunctor = new Connection::ConnFunctor(curConn,incoming);
//					while(m_server_running && ((tmpFunctor = pool->delegateFunctor(tmpFunctor))) != NULL  ){
//					  usleep(100);
//					}
//
//					if(tmpFunctor != NULL  ){
//					  modbus_ERROR_WRITE("Functor could not be added\n");
//					  delete tmpFunctor;
//					  curConn -> setStatus(MB_Framework::MBConnection::open);
//					}

					tmpFunctor->functor_function();
					delete tmpFunctor;
				} else {
					if (curConn->getStatus() == MB_Framework::MBConnection::wait)//reset connection without incoming data
					{
						curConn->setStatus(MB_Framework::MBConnection::open);//reset status
					}
				}
				++conn_it;
			}
		}
    }
}

} /* namespace MB_Gateway */
} /* namespace icke2063 */
