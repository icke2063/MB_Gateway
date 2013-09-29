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

//boost
#include <boost/serialization/singleton.hpp>
using namespace boost::serialization;

#include <modbus-tcp.h>

#include <SlaveList.h>
#include <MBHandlerInt.h>
#include <HandlerParam.h>

namespace icke2063 {
namespace MB_Gateway {

Connection::Connection(modbus_t *ctx) :
		m_connection_running(false), FunctorInt(false) {

	logger = &log4cpp::Category::getInstance(std::string("connection"));
	logger->setPriority(log4cpp::Priority::DEBUG);
	//if (console)logger->addAppender(console);

	logger->info("Connection\n");

	/* validate connection information and enable functor_function */
	if (ctx != NULL) {
		m_ctx = *ctx;
		m_connection_running = true;
		m_status = MBConnection::open;
	}
}

Connection::~Connection() {
	logger->info("~Connection\n");

	/* Connection closed by the client or error */
	if(m_connection_running){
		modbus_close(&m_ctx);
	}
}

bool Connection::handleQuery(uint8_t* query, VirtualRTUSlave* tmp_slave,
		enum handleQuery_mode mode) {

	/* var for query informations */
	int offset;
	uint8_t slave;
	uint8_t function;
	uint16_t address;
	uint16_t count = 0;

	uint16_t register_done = 0;
	uint16_t cur_address;
	uint16_t handler_retval;

	/* get needed informatons */
	offset = m_ctx.backend->header_length;
	slave = query[offset - 1];
	function = query[offset];
	address = (query[offset + 1] << 8) + query[offset + 2];
	cur_address = address;
	map<uint16_t,MBHandlerInt*> *cur_handlerlist = NULL;

	/* get data count */
	switch (function) {
	case _FC_READ_INPUT_REGISTERS:
		cur_handlerlist = &tmp_slave->m_input_handlerlist;
		break;
	case _FC_READ_HOLDING_REGISTERS:
	case _FC_WRITE_MULTIPLE_REGISTERS:
		cur_handlerlist = &tmp_slave->m_holding_handlerlist;
		count = (query[offset + 3] << 8) + query[offset + 4];
		break;
	case _FC_WRITE_SINGLE_REGISTER:
		cur_handlerlist = &tmp_slave->m_holding_handlerlist;
		count = 1;
		break;
		/* unsupported FC */
	case _FC_READ_COILS:
	case _FC_READ_DISCRETE_INPUTS:
	case _FC_WRITE_SINGLE_COIL:
	case _FC_READ_EXCEPTION_STATUS:
	case _FC_WRITE_MULTIPLE_COILS:
	case _FC_REPORT_SLAVE_ID:
	case _FC_WRITE_AND_READ_REGISTERS:
	default:
		return false;
		break;
	}

	logger->debug("count[0x%x]", count);

	/* loop over handlerlist */
	while (cur_address < (address + count)) {
		logger->debug("find handler[0x%x]", cur_address);
		logger->debug("handler size: %i", cur_handlerlist->size());

		if (cur_handlerlist->size() > 0) {
			/* get handlerfunction of current address */
			map<uint16_t, MBHandlerInt*>::iterator handler_it =
					cur_handlerlist->find(cur_address); //try to find slavehandlers
			if (handler_it != cur_handlerlist->end()) {
				logger->debug("found handler");
				HandlerParam * param = new HandlerParam(slave, function,
						cur_address, count - register_done,
						tmp_slave->getMappingDB()); //create new handler object
				switch (mode) {
				case handleReadAccess:
					logger->debug("handleReadAccess[0x%x]", cur_address);
					//call handleReadAccess function
					if ((handler_retval =
							(*handler_it).second->handleReadAccess(param))
							> 0) {
						logger->debug("handler_retval: %i", handler_retval);
						cur_address += handler_retval;
						register_done += handler_retval;
						continue;
					} else {
						//handler error -> exception
						logger->error("handler error: modbus_reply_exception");
						return false;
						break;
					}
					break;

				case checkWriteAccess:
					logger->debug("checkWriteAccess[0x%x]", cur_address);
					//call handleReadAccess function
					if ((handler_retval =
							(*handler_it).second->checkWriteAccess(param))
							> 0) {
						logger->debug("handler_retval: %i", handler_retval);
						cur_address += handler_retval;
						register_done += handler_retval;
						continue;
					} else {
						//handler error -> exception
						logger->error("handler error: modbus_reply_exception");
						return false;
						break;
					}
					break;
				case handleWriteAccess:
					logger->debug("handleWriteAccess[0x%x]", cur_address);
					//call handleReadAccess function
					if ((handler_retval =
							(*handler_it).second->handleWriteAccess(param))
							> 0) {
						logger->debug("handler_retval: %i", handler_retval);
						cur_address += handler_retval;
						register_done += handler_retval;
						continue;
					} else {
						//handler error -> exception
						logger->error("handler error: modbus_reply_exception");
						return false;
						break;
					}
					break;
				default:
					return 0;
					break;
				}
			} else {
				//no handler found -> exception
				logger->error("no handler found: modbus_reply_exception");
				return false;
				break;
			}
		} else {
			//no handler found -> exception
			logger->error("empty handlerlist: modbus_reply_exception");
			return false;
			break;
		}
	}
	return true;
}
void Connection::functor_function(void) {
	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
	int rc; /* mb return code */

	/* query informations */
	int offset;
	uint8_t slave;
	uint8_t function;

	/* Watch stdin (fd 0) to see when it has input. */

	logger->debug("functor_function\n");

	rc = modbus_receive(&m_ctx, query); /* receive mobus tcp query */
	logger->debug("modbus_receive:%i", rc);

	if (rc != -1) {

		offset = m_ctx.backend->header_length;
		slave = query[offset - 1];
		function = query[offset];

		logger->debug("query[slave]:0x%x", slave);
		logger->debug("query[function]:0x%x", function);
		{
			//lock list access
			boost::lock_guard<boost::mutex> lock(
					*(boost::serialization::singleton<SlaveList>::get_mutable_instance().getLock()->getMutex()));

			VirtualRTUSlave *tmp_slave =
					dynamic_cast<VirtualRTUSlave *>(boost::serialization::singleton<
							SlaveList>::get_mutable_instance().getSlave(slave));
			logger->debug("slave[0x%x]:0x%x", slave, tmp_slave);

			if (tmp_slave != NULL) {
				switch (function) {
				case _FC_READ_INPUT_REGISTERS:
				case _FC_READ_HOLDING_REGISTERS:
					/*
					 * read operations
					 * -> handle ReadAccess by handleQuery
					 */
					if (handleQuery(query, tmp_slave, handleReadAccess)) {
						rc = modbus_reply(&m_ctx, query, rc,
								tmp_slave->getMappingDB());
						logger->debug("modbus_reply[0x%x;0x%x]:0x%x", m_ctx.s,
								tmp_slave->getMappingDB(),
								rc);
					} else {
						modbus_reply_exception(&m_ctx, query,
								MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
					}
					break;
				case _FC_WRITE_MULTIPLE_REGISTERS:
				case _FC_WRITE_SINGLE_REGISTER:
					/*
					 * write operations
					 *  -> check writeAccess by handleQuery
					 *  -> extract data from query by modbus library to database
					 *  -> handle writeAccess by handleQuery(ThreadPool ?)
					 */
					if (handleQuery(query, tmp_slave, checkWriteAccess)) {
						rc = modbus_reply(&m_ctx, query, rc,tmp_slave->getMappingDB());
						logger->debug("modbus_reply[0x%x;0x%x]:0x%x", m_ctx.s,
								tmp_slave->getMappingDB(),
								rc);
						handleQuery(query, tmp_slave, handleWriteAccess);
					} else {
						modbus_reply_exception(&m_ctx, query,
								MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
					}
					break;
					/* unsupported FC */
				case _FC_READ_COILS:
				case _FC_READ_DISCRETE_INPUTS:
				case _FC_WRITE_SINGLE_COIL:
				case _FC_READ_EXCEPTION_STATUS:
				case _FC_WRITE_MULTIPLE_COILS:
				case _FC_REPORT_SLAVE_ID:
				case _FC_WRITE_AND_READ_REGISTERS:
				default:
					modbus_reply_exception(&m_ctx, query,
							MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
					break;
				}
			} else {
				logger->error("slave not registred: modbus_reply_exception");
				modbus_reply_exception(&m_ctx, query,
						MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);

			}
		}

		m_status = open;
	} else {
		m_status = closed;
	}
}

} /* namespace MB_Framework */
} /* namespace icke2063 */
