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
#include <exception>

#include <mb_common.h>

//boost
#include <boost/serialization/singleton.hpp>

#include <SlaveList.h>
#include <MBVirtualRTUSlave.h>
#include <MBHandlerInt.h>
#include <HandlerParam.h>

#include <modbus_logging_macros.h>

namespace icke2063 {
namespace MB_Gateway {

Connection::Connection(modbus_t *ctx) :
		m_connection_running(false) {

	modbus_NOTICE_WRITE("Connection\n");

	/* validate connection information and enable functor_function */
	if (ctx != NULL) {
		modbus_INFO_WRITE("MBSocket[%d]\n",modbus_get_socket(ctx));
		p_ctx = ctx;
		m_connection_running = true;
	} else {
		throw std::runtime_error("invalid handle");
	}
}

Connection::~Connection() {
	modbus_NOTICE_WRITE("~Connection\n");
	/* Connection closed by the client or error */
	m_connection_running = false;
	if(p_ctx){
		modbus_INFO_WRITE("~MBSocket[%d]\n",modbus_get_socket(p_ctx));
		modbus_close(p_ctx);
		modbus_free(p_ctx);
		p_ctx = NULL;
	}
}

bool Connection::handleQuery(uint8_t* query, std::shared_ptr<VirtualRTUSlave> tmp_slave,
		enum handleQuery_mode mode) 
{
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
	offset = modbus_get_header_length(p_ctx);

	slave = query[offset - 1];
	function = query[offset];
	address = (query[offset + 1] << 8) + query[offset + 2];
	cur_address = address;
	MB_Framework::m_handlerlist_type *cur_handlerlist = NULL;
	MB_Framework::m_blocklist_type *cur_blocklist = NULL;

	modbus_INFO_WRITE("function[0x%x]", function);
	modbus_INFO_WRITE("address[0x%x]", address);
	/* get data count */
	switch (function) {
	case _FC_READ_INPUT_REGISTERS:
		cur_handlerlist = &tmp_slave->m_input_handlerlist;
		cur_blocklist = &tmp_slave->m_input_blocklist;
		count = (query[offset + 3] << 8) + query[offset + 4];
		break;
	case _FC_READ_HOLDING_REGISTERS:
	case _FC_WRITE_MULTIPLE_REGISTERS:
		cur_handlerlist = &tmp_slave->m_holding_handlerlist;
		cur_blocklist = &tmp_slave->m_holding_blocklist;
		count = (query[offset + 3] << 8) + query[offset + 4];
		break;
	case _FC_WRITE_SINGLE_REGISTER:
		cur_handlerlist = &tmp_slave->m_holding_handlerlist;
		cur_blocklist = &tmp_slave->m_holding_blocklist;
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

	modbus_INFO_WRITE("count[0x%x]", count);

	std::shared_ptr<MB_Framework::MBHandlerInt> tmpHandler;

	/* loop over handlerlist */
	while (cur_address < (address + count))
	{
	    modbus_DEBUG_WRITE("search handler[0x%x]", cur_address);
	    modbus_DEBUG_WRITE("handler size: %i", cur_handlerlist->size());
	    tmpHandler.reset();
	    uint16_t cur_count = 0;
	    /**
	     * First we try directly to find a matching handlier within handlerlist
	     */
	    if ( cur_handlerlist != NULL && cur_handlerlist->size() > 0 )
	    {
		/* get handlerfunction of current address */
		MB_Framework::m_handlerlist_type::iterator handler_it =	cur_handlerlist->find(cur_address); //try to find slavehandlers
		if (handler_it != cur_handlerlist->end())
		{
		    tmpHandler = handler_it->second;
		    cur_count = count - register_done;
		}
	    }

	    if ( tmpHandler.get() == NULL )
	    {
		/**
		 * Now we try to find a matching handlier within blocklistlist
		 */
		modbus_DEBUG_WRITE("no handler found @%i", cur_address );
		modbus_DEBUG_WRITE("blocklist size %i", cur_blocklist->size());

		if (cur_blocklist != NULL && cur_blocklist->size() > 0 )
		{
		    for ( auto block_it = cur_blocklist->begin(); block_it != cur_blocklist->end(); block_it++)
		    {
			if ( (*block_it).get() != NULL )
			{
			    modbus_DEBUG_WRITE("block[%i..%i]", (*block_it)->getStartAddr(),
				     (*block_it)->getEndAddr());

			    if ( (*block_it)->getStartAddr()<= cur_address 
				    &&  (*block_it)->getEndAddr() >= cur_address )
			    {
				tmpHandler = (*block_it)->getHandler();
				cur_count = count - register_done;
			        if ( (cur_address + cur_count - 1) > (*block_it)->getEndAddr() )
				{
				    cur_count = (*block_it)->getEndAddr() - cur_count + 1;
				}
				modbus_DEBUG_WRITE("curcount[%i]", cur_count );

				if ( cur_count <= 0 ){ return false; }
			    }
			}
		    }
		}
	    }

	    if( tmpHandler.get() != NULL )
	    {
				modbus_DEBUG_WRITE("found handler");
				HandlerParam * param = new HandlerParam(slave, function,
						cur_address, cur_count,
						tmp_slave->getMappingDB()); //create new handler object
				switch (mode) {
				case handleReadAccess:
					modbus_DEBUG_WRITE("handleReadAccess[0x%x]", cur_address);
					//call handleReadAccess function
					if ((handler_retval = tmpHandler->handleReadAccess(param)) > 0) {
						modbus_DEBUG_WRITE("handler_retval: %i", handler_retval);
						cur_address += handler_retval;
						register_done += handler_retval;
						continue;
					} else {
						//handler error -> exception
						modbus_WARN_WRITE("no handler result: modbus_reply_exception");
						return false;
						break;
					}
					break;

				case checkWriteAccess:
					modbus_DEBUG_WRITE("checkWriteAccess[0x%x]", cur_address);
					//call handleReadAccess function
					if ((handler_retval = tmpHandler->checkWriteAccess(param)) > 0) {
						modbus_DEBUG_WRITE("handler_retval: %i", handler_retval);
						cur_address += handler_retval;
						register_done += handler_retval;
						continue;
					} else {
						//handler error -> exception
						modbus_WARN_WRITE("no handler result: modbus_reply_exception");
						return false;
						break;
					}
					break;
				case handleWriteAccess:
					modbus_DEBUG_WRITE("handleWriteAccess[0x%x]", cur_address);
					//call handleReadAccess function
					if ((handler_retval = tmpHandler->handleWriteAccess(param)) > 0) {
						modbus_DEBUG_WRITE("handler_retval: %i", handler_retval);
						cur_address += handler_retval;
						register_done += handler_retval;
						continue;
					} else {
						//handler error -> exception
						modbus_WARN_WRITE("no handler result: modbus_reply_exception");
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
				modbus_WARN_WRITE("no handler found: modbus_reply_exception");
				return false;
				break;
			}
	}
	return true;
}
void Connection::ConnFunctor::functor_function(void) {
	std::shared_ptr<Connection> tmp_conn;
	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
	int rc; /* mb return code */

	/* query informations */
	modbus_t *p_ctx;
	int offset;
	uint8_t slave;
	uint8_t function;

	
	if(! (( tmp_conn = wp_conn.lock() )) ){
	    modbus_ERROR_WRITE("functor_function: wp_conn failure\n");
	  return;								//check existence of parent class
	}
	p_ctx = tmp_conn->getConnInfo();
	if(p_ctx == NULL){
	    modbus_ERROR_WRITE("functor_function: p_ctx failure\n");
	    return;
	}

	/* Watch stdin (fd 0) to see when it has input. */

	modbus_DEBUG_WRITE("functor_function\n");

	rc = modbus_receive(p_ctx, query); /* receive mobus tcp query */
	modbus_DEBUG_WRITE("modbus_receive[%d]:%i",modbus_get_socket(p_ctx), rc);

	if (rc != -1) {

		offset = modbus_get_header_length(p_ctx);

		slave = query[offset - 1];				//get slaveID
		function = query[offset];				//get mb function code

		modbus_INFO_WRITE("query[slave]:0x%x", slave);
		modbus_DEBUG_WRITE("query[function]:0x%x", function);
		{
			///lock list access @todo use shared lock
			//std::lock_guard<std::mutex> lock(*(boost::serialization::singleton<SlaveList>::get_mutable_instance().getLock()->getMutex().get()));

			std::shared_ptr<VirtualRTUSlave> tmp_slave =
					std::dynamic_pointer_cast<VirtualRTUSlave>(
							boost::serialization::singleton<SlaveList>::get_mutable_instance().getSlave(slave));
			modbus_DEBUG_WRITE("slave[0x%x]:0x%x", slave, tmp_slave.get());



			if (tmp_slave.get() != NULL) {
				switch (function) {
				case _FC_READ_INPUT_REGISTERS:
				case _FC_READ_HOLDING_REGISTERS:
					/*
					 * read operations
					 * -> handle ReadAccess by handleQuery
					 */
					if (tmp_conn->handleQuery(query, tmp_slave, handleReadAccess)) {
						rc = modbus_reply(p_ctx, query, rc,	tmp_slave->getMappingDB());
						modbus_DEBUG_WRITE("modbus_reply[%d;0x%x]:%d", modbus_get_socket(p_ctx),
								tmp_slave->getMappingDB(), rc);
					} else {
						modbus_WARN_WRITE("handler error: modbus_reply_exception");
						modbus_reply_exception(p_ctx, query, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
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
					if (tmp_conn->handleQuery(query, tmp_slave, checkWriteAccess)) {
						rc = modbus_reply(p_ctx, query, rc,tmp_slave->getMappingDB());
						modbus_DEBUG_WRITE("modbus_reply[0x%x;0x%x]:0x%x", modbus_get_socket(p_ctx),
								tmp_slave->getMappingDB(), rc);
						tmp_conn->handleQuery(query, tmp_slave, handleWriteAccess);
					} else {
						modbus_WARN_WRITE("handler error: modbus_reply_exception");
						modbus_reply_exception(p_ctx, query, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
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
					modbus_WARN_WRITE("function not registred: modbus_reply_exception");
					modbus_reply_exception(p_ctx, query, MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
					break;
				}
			} else {
				modbus_WARN_WRITE("slave not registred: modbus_reply_exception");
				modbus_reply_exception(p_ctx, query, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);

			}
		}

		tmp_conn->setStatus(open);
	} else {
		tmp_conn->setStatus(closed);
	}
}

} /* namespace MB_Framework */
} /* namespace icke2063 */
