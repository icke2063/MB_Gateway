/**
 * @file   MultiRegisterHandler.cpp
 * @Author icke
 * @date   03.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include "handler/MultiRegisterHandler.h"
#include <HandlerParam.h>
#include "modbus-private.h"
#include <string.h>

namespace icke2063 {
namespace MB_Gateway {

int MultiRegisterHandler::handleReadAccess(MBHandlerParam *param) {
	uint16_t address;
	uint16_t i2c_address;
	uint16_t register_count;

	logger->debug("MultiRegisterHandler::handleReadAccess");

	HandlerParam *curHandler = dynamic_cast<HandlerParam*>(param);
	if (curHandler != NULL) { //got correct param object pointer
		logger->debug("HandlerParam ok");

		address = curHandler->m_address;

		logger->debug("slave:%i", curHandler->m_slave);
		logger->debug("function:%i", curHandler->m_function);

		logger->debug("p_mb_mapping:0x%x", curHandler->p_mb_mapping);
		logger->debug("address:%i", address);

		if (m_register_count < 0)
			register_count = curHandler->m_count;
		else
			register_count = m_register_count;

		logger->debug("count:%i", curHandler->m_count);
		logger->debug("register_count:%i", register_count);

		//check FC support and storage size
		switch (curHandler->m_function) {
		case _FC_READ_INPUT_REGISTERS: //FC 03:read holding register
			if (!enableReadInpReg) {
				logger->error("enableReadInpReg: false");
				return 0;
			}
			if ((address + register_count)
					> curHandler->p_mb_mapping->nb_input_registers) {
				logger->error("mapping size failure");
				return 0;
			}
			break;
		case _FC_READ_HOLDING_REGISTERS: //FC 04:read input register
			if (!enableReadHolReg) {
				logger->error("enableReadHolReg: false");
				return 0;
			}
			if ((address + register_count)
					> curHandler->p_mb_mapping->nb_registers) {
				logger->error("mapping size failure");
				return 0;
			}
			break;
		default: //not supported FC in this handler
			logger->error("not supported FC");
			return 0;
		}

		//copy data to mb_mapping
		switch (curHandler->m_function) {
		case _FC_READ_INPUT_REGISTERS: //FC 03:read holding register
		{
			memcpy(&curHandler->p_mb_mapping->tab_input_registers[address],
					&p_mb_mapping->tab_input_registers[address],
					register_count);
		}
			break;
		case _FC_READ_HOLDING_REGISTERS: //FC 04:read input register
		{
			memcpy(&curHandler->p_mb_mapping->tab_registers[address],
					&p_mb_mapping->tab_registers[address], register_count);
		}
			break;
		default: //not supported FC in this handler
			return 0;
		}
		return register_count;

	}
	return 0; //return zero register handled > modbus exception
}

int MultiRegisterHandler::handleWriteAccess(MBHandlerParam *param) {
	uint8_t slave;
	uint16_t address;
	uint16_t register_count;
	uint8_t send_offset = 0, i;

	logger->debug("MultiByteHandler::handleWriteAccess");
	HandlerParam *curHandler = dynamic_cast<HandlerParam*>(param);
	if (curHandler != NULL) { //got correct param object pointer
		if (m_register_count < 0)
			register_count = curHandler->m_count;
		else
			register_count = m_register_count;

		slave = curHandler->m_slave;
		address = curHandler->m_address;

		logger->debug("slave:%i", slave);
		logger->debug("function:%i", curHandler->m_function);
		logger->debug("count:%i", curHandler->m_count);
		logger->debug("register_count:%i", register_count);
		logger->debug("address:%i", address);

		logger->debug("p_mb_mapping:%x", curHandler->p_mb_mapping);

		switch (curHandler->m_function) {
		case _FC_WRITE_SINGLE_REGISTER: //FC 06:write single holding register
			if (!enableWriteSHolReg)
				return 0;
			break;
		case _FC_WRITE_MULTIPLE_REGISTERS: //FC 16:write multiple holding register
		{
			if (!enableWriteHolReg)
				return 0;
			if ((address + register_count)
					> curHandler->p_mb_mapping->nb_registers)
				return 0;

			memcpy(&p_mb_mapping->tab_registers[address],
					&curHandler->p_mb_mapping->tab_registers[address],
					register_count);

			return register_count;
		}
			break;
		default: //not supported FC in this handler
			return 0;
		}
	}
	return 0; //return zero register handled > modbus exception
}

int MultiRegisterHandler::checkWriteAccess(MBHandlerParam *param) {
	uint8_t slave;
	uint16_t address;
	uint16_t register_count;

	logger->debug("MultiRegisterHandler::checkWriteAccess");
	HandlerParam *curHandler = dynamic_cast<HandlerParam*>(param);
	if (curHandler != NULL) { //got correct param object pointer
		if (m_register_count < 0)
			register_count = curHandler->m_count;
		else
			register_count = m_register_count;

		return register_count;
	}
	return 0;
}
} /* namespace MB_Gateway */
} /* namespace icke2063 */
