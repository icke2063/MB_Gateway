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

#include <mb_common.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

int MultiByteHandler::handleReadAccess(MB_Framework::MBHandlerParam *param) {
	uint16_t address;
	uint16_t i2c_address;
	uint16_t byte_count;

	i2c_DEBUG_WRITE("MultiByteHandler::handleReadAccess[0x%x]", m_mode);

	HandlerParam *curHandler = dynamic_cast<HandlerParam*>(param);
	if (curHandler != NULL) { //got correct param object pointer
		i2c_DEBUG_WRITE("HandlerParam ok");

		address = curHandler->m_address;
		i2c_address = 2 * address;

		i2c_DEBUG_WRITE("slave:%i", curHandler->m_slave);
		i2c_DEBUG_WRITE("function:%i", curHandler->m_function);

		i2c_DEBUG_WRITE("p_mb_mapping:0x%x", curHandler->p_mb_mapping);
		i2c_DEBUG_WRITE("address:%i", address);
		i2c_DEBUG_WRITE("I2C Address:0x%x", i2c_address);

		if (m_byte_count < 0)
			byte_count = curHandler->m_count * 2;
		else
			byte_count = m_byte_count;

		i2c_DEBUG_WRITE("count:%i", curHandler->m_count);
		i2c_DEBUG_WRITE("byte_count:%i", byte_count);

		//check FC support and storage size
		switch (curHandler->m_function) {
		case _FC_READ_INPUT_REGISTERS: //FC 03:read holding register
			if (!enableReadInpReg) {
				i2c_ERROR_WRITE("enableReadInpReg: false");
				return 0;
			}
			if (address + (byte_count / 2)
					> curHandler->p_mb_mapping->nb_input_registers) {
				i2c_ERROR_WRITE("mapping size failure");
				return 0;
			}
			break;
		case _FC_READ_HOLDING_REGISTERS: //FC 04:read input register
			if (!enableReadHolReg) {
				i2c_ERROR_WRITE("enableReadHolReg: false");
				return 0;
			}
			if (address + (byte_count / 2)
					> curHandler->p_mb_mapping->nb_registers) {
				i2c_ERROR_WRITE("mapping size failure");
				return 0;
			}
			break;
		default: //not supported FC in this handler
			i2c_ERROR_WRITE("not supported FC");
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
				i2c_ERROR_WRITE("8Bit mode[%i]", i2c_address);
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

			icke2063::common_cpp::Convert converter;
			//copy data to mb_mapping
			switch (curHandler->m_function) {
			case _FC_READ_INPUT_REGISTERS: //FC 03:read holding register
			{
				converter.BigEndiantoShort(
						&curHandler->p_mb_mapping->tab_input_registers[address],
						(uint16_t*)recvbuffer, (size_t) curHandler->m_count);
//				int i;
//				for(i=0;i<byte_count;i++){
//					i2c_DEBUG_WRITE("recv[%i]:0x%x",i,recvbuffer[i]);
//				}
			}
				break;
			case _FC_READ_HOLDING_REGISTERS: //FC 04:read input register
			{
				converter.BigEndiantoShort(
						&curHandler->p_mb_mapping->tab_registers[address],
						(uint16_t*)recvbuffer, (size_t) curHandler->m_count);
//				int i;
//				for(i=0;i<byte_count;i++){
//					i2c_DEBUG_WRITE("recv[%i]:0x%x",i,recvbuffer[i]);
//				}
			}
				break;
			default: //not supported FC in this handler
				return 0;
			}
			return ((byte_count % 2) == 0) ?
					byte_count / 2 : (byte_count / 2) + 1;
		} else
			i2c_ERROR_WRITE("Read_I2C_Bytes failure");
	}
	return 0; //return zero register handled > modbus exception
}

int MultiByteHandler::handleWriteAccess(MB_Framework::MBHandlerParam *param) {
	uint8_t slave;
	uint16_t address;
	uint16_t i2c_address;
	uint16_t byte_count;
	uint8_t send_offset = 0;
	uint16_t ret = 0;

	i2c_DEBUG_WRITE("MultiByteHandler::handleWriteAccess[%i]", m_mode);
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

		i2c_DEBUG_WRITE("slave:%i", slave);
		i2c_DEBUG_WRITE("function:%i", curHandler->m_function);
		i2c_DEBUG_WRITE("count:%i", curHandler->m_count);
		i2c_DEBUG_WRITE("byte_count:%i", byte_count);
		i2c_DEBUG_WRITE("address:%i", address);
		i2c_DEBUG_WRITE("i2c_address:%i", i2c_address);

		i2c_DEBUG_WRITE("p_mb_mapping:%x", curHandler->p_mb_mapping);

		/**
		 * @todo path by config or cmdline param
		 */
		boost::serialization::singleton<I2C_Comm>::get_mutable_instance().i2cOpen(
				"/dev/i2c-1");

		//write i2c register address -> sendbuffer (depending on address mode)
		switch (m_mode) {
		case _8bit: //8 bit address mode -> i2c_address should be under 0xff
			if (i2c_address > 0xff) {
				i2c_ERROR_WRITE("8Bit mode[%i]", i2c_address);
				return 0;
			}
			i2c_DEBUG_WRITE("8Bit mode[0x%x]", i2c_address);
			sendbuffer[send_offset++] = (i2c_address & 0xff); //only low value
			break;
		case _16bit:
			i2c_DEBUG_WRITE("16Bit mode[0x%x]", i2c_address);
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
				i2c_ERROR_WRITE("WriteSingleHolReg disabled");
				return 0;
			}
			break;
		case _FC_WRITE_MULTIPLE_REGISTERS: //FC 16:write multiple holding register
		{
			if (!enableWriteHolReg){
				i2c_ERROR_WRITE("WriteHilReg disabled");
				return 0;
			}
			if ((address + (byte_count/2)) > curHandler->p_mb_mapping->nb_registers){
				i2c_ERROR_WRITE("out of range: 0x%x > 0x%x",(address + (byte_count/2)),curHandler->p_mb_mapping->nb_registers);
				return 0;
			}
		}
			break;
		default: //not supported FC in this handler
			i2c_ERROR_WRITE("FC 0x%x not supported",curHandler->m_function);
			return 0;
		}

		icke2063::common_cpp::Convert converter;
		converter.ShorttoBigEndian((uint16_t*)&sendbuffer[send_offset],
				&curHandler->p_mb_mapping->tab_registers[address],
				(size_t) curHandler->m_count);

		if (boost::serialization::singleton<I2C_Comm>::get_mutable_instance().Write_I2C_Bytes(
				slave, sendbuffer, m_mode, byte_count)){
			ret = ((byte_count % 2) == 0) ?
					byte_count / 2 : (byte_count / 2) + 1;
			i2c_DEBUG_WRITE("written data count: %i",ret);
			return ret;
		}
		else
			i2c_ERROR_WRITE("Write_I2C_Bytes failure");

	}
	return 0; //return zero register handled > modbus exception
}

int MultiByteHandler::checkWriteAccess(MB_Framework::MBHandlerParam *param) {
	uint16_t byte_count;

	i2c_DEBUG_WRITE("MultiByteHandler::checkWriteAccess");
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
