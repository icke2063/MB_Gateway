/**
 * @file   MultiByteHandler.cpp
 * @Author icke
 * @date   13.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include "MultiByteHandler.h"

#include <HandlerParam.h>
#include "boost/serialization/singleton.hpp"
#include <I2CComm.h>
#include <string.h>
#include <Convert.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

int MultiByteHandler::handleReadAccess(MBHandlerParam *param) {
	uint16_t address;
	uint16_t i2c_address;
	uint16_t byte_count;

	logger->debug("MultiByteHandler::handleReadAccess[0x%x]", m_mode);

	HandlerParam *curHandler = dynamic_cast<HandlerParam*>(param);
	if (curHandler != NULL) { //got correct param object pointer
		logger->debug("HandlerParam ok");

		address = curHandler->m_address;
		i2c_address = 2 * address;

		logger->debug("slave:%i", curHandler->m_slave);
		logger->debug("function:%i", curHandler->m_function);

		logger->debug("p_mb_mapping:0x%x", curHandler->p_mb_mapping);
		logger->debug("address:%i", address);
		logger->debug("I2C Address:0x%x", i2c_address);

		if (m_byte_count < 0)
			byte_count = curHandler->m_count * 2;
		else
			byte_count = m_byte_count;

		logger->debug("count:%i", curHandler->m_count);
		logger->debug("byte_count:%i", byte_count);

		//check FC support and storage size
		switch (curHandler->m_function) {
		case _FC_READ_INPUT_REGISTERS: //FC 03:read holding register
			if (!enableReadInpReg) {
				logger->error("enableReadInpReg: false");
				return 0;
			}
			if (address + (byte_count / 2)
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
			if (address + (byte_count / 2)
					> curHandler->p_mb_mapping->nb_registers) {
				logger->error("mapping size failure");
				return 0;
			}
			break;
		default: //not supported FC in this handler
			logger->error("not supported FC");
			return 0;
		}

		/*
		 * address register (m_mode byte)
		 * data register (byte_count byte)
		 */
		uint8_t recvbuffer[m_mode + byte_count];

		/**
		 * @todo path by config or cmdline param
		 */

		boost::serialization::singleton<I2C_Comm>::get_mutable_instance().i2cOpen(
				"/dev/i2c-1");

		//write i2c register address -> recvbuffer (depending on address mode)
		switch (m_mode) {
		case _8bit: //8 bit address mode -> i2c_address should be under 0xff
			if (i2c_address > 0xff) {
				logger->error("8Bit mode[%i]", i2c_address);
				return 0;
			}
			recvbuffer[0] = (i2c_address & 0xff); //only low value
			break;
		case _16bit:
			recvbuffer[0] = (i2c_address >> 8); //first high
			recvbuffer[1] = (i2c_address & 0xff); //second low
			break;
		default:
			return 0;
			break;
		}

		// read data from i2c bus
		if (boost::serialization::singleton<I2C_Comm>::get_mutable_instance().Read_I2C_Bytes(
				curHandler->m_slave, recvbuffer, m_mode, byte_count)) {

			Convert converter;
			//copy data to mb_mapping
			switch (curHandler->m_function) {
			case _FC_READ_INPUT_REGISTERS: //FC 03:read holding register
			{
				converter.BigEndiantoShort(
						&curHandler->p_mb_mapping->tab_input_registers[address],
						recvbuffer, (size_t) curHandler->m_count);
//				int i;
//				for(i=0;i<byte_count;i++){
//					logger->debug("recv[%i]:0x%x",i,recvbuffer[i]);
//				}
			}
				break;
			case _FC_READ_HOLDING_REGISTERS: //FC 04:read input register
			{
				converter.BigEndiantoShort(
						&curHandler->p_mb_mapping->tab_registers[address],
						recvbuffer, (size_t) curHandler->m_count);
//				int i;
//				for(i=0;i<byte_count;i++){
//					logger->debug("recv[%i]:0x%x",i,recvbuffer[i]);
//				}
			}
				break;
			default: //not supported FC in this handler
				return 0;
			}
			return ((byte_count % 2) == 0) ?
					byte_count / 2 : (byte_count / 2) + 1;
		} else
			logger->error("Read_I2C_Bytes failure");
	}
	return 0; //return zero register handled > modbus exception
}

int MultiByteHandler::handleWriteAccess(MBHandlerParam *param) {
	uint8_t slave;
	uint16_t address;
	uint16_t i2c_address;
	uint16_t byte_count;
	uint8_t send_offset = 0;
	uint16_t ret = 0;

	logger->debug("MultiByteHandler::handleWriteAccess[%i]", m_mode);
	HandlerParam *curHandler = dynamic_cast<HandlerParam*>(param);
	if (curHandler != NULL) { //got correct param object pointer
		if (m_byte_count < 0)
			byte_count = curHandler->m_count * 2;
		else
			byte_count = m_byte_count;

		/*
		 * address register (2 byte)
		 * data register (2 byte)
		 */
		uint8_t sendbuffer[m_mode + byte_count];

		slave = curHandler->m_slave;
		address = curHandler->m_address;
		i2c_address = address * 2;

		logger->debug("slave:%i", slave);
		logger->debug("function:%i", curHandler->m_function);
		logger->debug("count:%i", curHandler->m_count);
		logger->debug("byte_count:%i", byte_count);
		logger->debug("address:%i", address);
		logger->debug("i2c_address:%i", i2c_address);

		logger->debug("p_mb_mapping:%x", curHandler->p_mb_mapping);

		/**
		 * @todo path by config or cmdline param
		 */
		boost::serialization::singleton<I2C_Comm>::get_mutable_instance().i2cOpen(
				"/dev/i2c-1");

		//write i2c register address -> sendbuffer (depending on address mode)
		switch (m_mode) {
		case _8bit: //8 bit address mode -> i2c_address should be under 0xff
			if (i2c_address > 0xff) {
				logger->error("8Bit mode[%i]", i2c_address);
				return 0;
			}
			logger->debug("8Bit mode[0x%x]", i2c_address);
			sendbuffer[send_offset++] = (i2c_address & 0xff); //only low value
			break;
		case _16bit:
			logger->debug("16Bit mode[0x%x]", i2c_address);
			sendbuffer[send_offset++] = (i2c_address >> 8); //first high
			sendbuffer[send_offset++] = (i2c_address & 0xff); //second low
			break;
		default:
			return 0;
			break;
		}

		switch (curHandler->m_function) {
		case _FC_WRITE_SINGLE_REGISTER: //FC 06:write single holding register
			if (!enableWriteSHolReg){
				logger->error("WriteSingleHolReg disabled");
				return 0;
			}
			break;
		case _FC_WRITE_MULTIPLE_REGISTERS: //FC 16:write multiple holding register
		{
			if (!enableWriteHolReg){
				logger->error("WriteHilReg disabled");
				return 0;
			}
			if ((address + (byte_count/2)) > curHandler->p_mb_mapping->nb_registers){
				logger->error("out of range: 0x%x > 0x%x",(address + (byte_count/2)),curHandler->p_mb_mapping->nb_registers);
				return 0;
			}
		}
			break;
		default: //not supported FC in this handler
			logger->error("FC 0x%x not supported",curHandler->m_function);
			return 0;
		}

		Convert converter;
		converter.ShorttoBigEndian(&sendbuffer[send_offset],
				&curHandler->p_mb_mapping->tab_registers[address],
				(size_t) curHandler->m_count);

		if (boost::serialization::singleton<I2C_Comm>::get_mutable_instance().Write_I2C_Bytes(
				slave, sendbuffer, m_mode, byte_count)){
			ret = ((byte_count % 2) == 0) ?
					byte_count / 2 : (byte_count / 2) + 1;
			logger->debug("written data count: %i",ret);
			return ret;
		}
		else
			logger->error("Write_I2C_Bytes failure");

	}
	return 0; //return zero register handled > modbus exception
}

int MultiByteHandler::checkWriteAccess(MBHandlerParam *param) {
	uint16_t byte_count;

	logger->debug("MultiByteHandler::checkWriteAccess");
	HandlerParam *curHandler = dynamic_cast<HandlerParam*>(param);
	if (curHandler != NULL) { //got correct param object pointer
		if (m_byte_count < 0)
			byte_count = curHandler->m_count * 2;
		else
			byte_count = m_byte_count;

		return ((byte_count % 2) == 0) ? byte_count / 2 : (byte_count / 2) + 1;
	}
	return 0;
}

} /* namespace I2C */
}/* namespace MB_Gateway */
}/* namespace icke2063 */
