/**
 * @file   Connection.cpp
 * @Author icke2063
 * @date   01.06.2013
 * @brief  Implementation of MBConnection class:
 * 			- log4cpp for logging
 * 			- use libmodbus as modbus tcp library
 * 			- use boost for singleton template at slavelist
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

#include "Connection.h"

#include <iostream>
#include "stddef.h"
#include <sys/select.h>
#include <errno.h>
#include <string.h>

// libmodbus
#include "modbus-private.h"
#include "modbus.h"

//boost
#include <boost/serialization/singleton.hpp>
using namespace boost::serialization;

#include <SlaveList.h>
using namespace MB_Framework;

namespace MB_Gateway {

Connection::Connection(modbus_t *ctx) :
		m_connection_running(false) {

	logger = &log4cpp::Category::getInstance(std::string("connection"));
	logger->setPriority(log4cpp::Priority::DEBUG);
	if (console)
		logger->addAppender(console);

	logger->info("Connection\n");

	/* validate connection information and enable functor_function */
	if (ctx != NULL) {
		m_ctx = *ctx;
		m_connection_running = true;
	}
}

Connection::~Connection() {
	logger->info("~Connection\n");
	m_connection_running = false;	//disable functor
}

void Connection::functor_function(void) {
	modbus_mapping_t * cur_mapping = NULL;
	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
	int rc; /* mb returen code */

	/* query informaions */
	int offset;
	uint8_t slave;
	uint8_t function;
	uint16_t count;
	uint16_t address;

	bool connected = true;

	/* select */
	fd_set rfds;
	struct timeval tv;
	int retval;

	uint16_t cur_address;
	uint16_t register_done;
	uint16_t handler_retval;

	cur_mapping = modbus_mapping_new(0, 0, 0, 0);

	/* Watch stdin (fd 0) to see when it has input. */

	logger->debug("functor_function\n");

	while (m_connection_running && connected) {

		FD_ZERO(&rfds);
		FD_SET(m_ctx.s, &rfds);

		/* Wait up to 500 useconds. */
		tv.tv_sec = 0;
		tv.tv_usec = 500;

		try {
			retval = select(m_ctx.s + 1, &rfds, NULL, NULL, &tv);
		} catch (const std::exception& e) {
			logger->error("select exception %s", e.what());
			continue;
		}

		if (retval == 0) {
			continue;
		}

		if (retval < 0) {
			logger->error("poll error:%d - %s\n", retval, strerror(errno));
			continue;
		}

		rc = modbus_receive(&m_ctx, query); /* receive mobus tcp query */
		logger->debug("modbus_receive:%i", rc);

		if (rc != -1) {

			offset = m_ctx.backend->header_length;
			slave = query[offset - 1];
			function = query[offset];
			count = (query[offset + 3] << 8) + query[offset + 4];
			address = (query[offset + 1] << 8) + query[offset + 2];

			cur_address = address;

			logger->debug("query[slave]:%i", slave);
			logger->debug("query[function]:%i", function);
			logger->debug("query[address]:%i", address);
			logger->debug("query[count]:%i", count);

			{
				//lock list access
				boost::lock_guard<boost::mutex> lock(
						boost::serialization::singleton<SlaveList>::get_mutable_instance().p_slavelist_lock->getMutex());
				//get pointer to list
				map<uint8_t, MBVirtualRTUSlave*> *p_slavelist =
						&(boost::serialization::singleton<SlaveList>::get_mutable_instance().slavelist);
				logger->debug("SlaveList size:%i", p_slavelist->size());

				map<uint8_t, MBVirtualRTUSlave*>::iterator slave_it =
						p_slavelist->find(slave); //try to find virtual RTU Slave

				if (slave_it != p_slavelist->end()) {

					/* loop over handlerlist */
					while (cur_address < (address + count)) {

						/* get handlerfunction of current address */
						map<uint16_t, MBHandlerInt*>::iterator handler_it =
								(*slave_it).second->handlerlist.find(
										cur_address); //try to find slavehandlers
						if (handler_it
								!= (*slave_it).second->handlerlist.end()) {

							//found handler function
							if ((handler_retval =
									(*handler_it).second->handleQuery()) > 0) {
								cur_address += handler_retval;
							} else {
								//handler error -> exception
								logger->error(
										"handler error: modbus_reply_exception");
								modbus_reply_exception(&m_ctx, query,
										MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
								continue;
							}
						} else {
							//no handler found -> exception
							logger->error(
									"no handler found: modbus_reply_exception");
							modbus_reply_exception(&m_ctx, query,
									MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
							continue;
						}
					}
				} else {
					logger->error("no slave found: modbus_reply_exception");
					//slave not found -> reply exception
					modbus_reply_exception(&m_ctx, query,
							MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
					continue;
				}

			}
			modbus_reply(&m_ctx, query, rc, cur_mapping);
		} else {
			connected = false;
			break;
		}
	}

	/* Connection closed by the client or error */
	modbus_close(&m_ctx);
}

} /* namespace MB_Framework */
