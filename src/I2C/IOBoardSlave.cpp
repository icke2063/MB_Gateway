/**
 * @file   IOBoardSlave.cpp
 * @Author icke
 * @date   02.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include <IOBoardSlave.h>
#include <stddef.h>
#include <string.h>

#include <HandlerList.h>
#include <I2CComm.h>

#include "boost/serialization/singleton.hpp"
#include <boost/iterator/iterator_concepts.hpp>
#include "MultiByteHandler.h"
#include "SingleRegisterHandler.h"
#include "HolRegHandler.h"
#include "HolRegHandlerRO.h"

#ifndef ICKE2063_CRUMBY_NO_CPP11
#include <thread>
#else
#include <boost/thread/locks.hpp>
#define nullptr NULL
#endif

using namespace IOBOARDSLAVE_H_NS;

#include <modbus_logging_macros.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

VersionHandler::VersionHandler() :
		MultiByteHandler(_16bit, -1) {
	modbus_INFO_WRITE("VersionHandler");
	enableReadInpReg = false; //disable input register access
}

int VersionHandler::handleReadAccess(MB_Framework::MBHandlerParam *param) {

	uint8_t count = ((VERSION_LENGTH / 2) + (VERSION_LENGTH % 2));

	modbus_DEBUG_WRITE("VersionHandler::handleReadAccess");

	HandlerParam *curHandler = dynamic_cast<HandlerParam*>(param);
	if (curHandler != NULL) {
		/* check for the correct address range */
		setRange((VERSION_START / 2), ((VERSION_START / 2) + count)-1);
		if (!checkRange((curHandler->m_address), curHandler->m_count)){
			modbus_ERROR_WRITE("checkRange: failure");
			return 0;
		}
		if (curHandler->m_count != count){
			modbus_ERROR_WRITE("Count to low for correct version readout: %i of %i",curHandler->m_count,count);
			return 0;
		}
		return MultiByteHandler::handleReadAccess(param);
	}
	return 0; //return zero register handled > modbus exception
}

int DataHandler::handleReadAccess(MB_Framework::MBHandlerParam *param) {
	modbus_DEBUG_WRITE("DataHandler::handleReadAccess");

	HandlerParam *curHandler = dynamic_cast<HandlerParam*>(param);
	if (curHandler != NULL) {
		//check access alignment
		if (curHandler->m_address % 2 != 0) {
			modbus_ERROR_WRITE("address not correct aligned");
			return 0; //only even adresses -> alignment 2 register
		}
		if (curHandler->m_count % 2) {
			modbus_ERROR_WRITE("count not correct aligned");
			return 0; //only even counts -> alignement 2 register count
		}

		/* check for the correct address range */
		if (!checkRange((curHandler->m_address), curHandler->m_count)) {
			modbus_ERROR_WRITE("range failure");
			modbus_ERROR_WRITE("Range(0x%x,0x%x)", getRStart(), getRStop());
			modbus_ERROR_WRITE("checkRange(0x%x,0x%x)", curHandler->m_address,
					curHandler->m_count);
			return 0;
		}

		return MultiByteHandler::handleReadAccess(param);
	}
	return 0; //return zero register handled > modbus exception
}
int DataHandler::checkWriteAccess(MB_Framework::MBHandlerParam *param) {
	modbus_DEBUG_WRITE("DataHandler::checkWriteAccess");

	HandlerParam *curHandler = dynamic_cast<HandlerParam*>(param);
	if (curHandler != NULL) {
		//check access alignment
		if (curHandler->m_address % 2 != 0)
			return 0; //only even adresses -> alignment 2 register
		if (curHandler->m_count % 2)
			return 0; //only even counts -> alignement 2 register count

		/* check for the correct address range */
		setRange((VIRTUAL_DATA_START / 2),
				(VIRTUAL_DATA_START / 2) + (IOBOARD_MAX_IO_PINS / 2) - 1);
		if (!checkRange((curHandler->m_address), curHandler->m_count))
			return 0;

		return MultiByteHandler::checkWriteAccess(param);
	}
	return 0; //return zero register handled > modbus exception
}

IOBoard_Slave::IOBoard_Slave(uint8_t SlaveAddr) :
		I2C_Slave(SlaveAddr)
{

	modbus_INFO_WRITE("IOBoard_Slave[0x%x]", SlaveAddr);

	if(m_mapping)modbus_mapping_free(m_mapping);
	m_mapping = modbus_mapping_new(0, 0, (I2C_BUFFER_SIZE + EEPROM_SIZE / 2),
			0);
	modbus_DEBUG_WRITE("m_mapping:%x", m_mapping);

	getSlaveInfo();
}

IOBoard_Slave::~IOBoard_Slave() {
}

bool IOBoard_Slave::init(void) {
	int i, e;
	uint8_t pincount;
	uint16_t handler_address;

	/*
	 * address register (m_mode byte)
	 * data register (byte_count byte)
	 */
	uint8_t recvbuffer[_16bit + 2];

	modbus_INFO_WRITE("init");

	/*
	 * get some informations directly from ioboard
	 */

	boost::serialization::singleton<I2C_Comm>::get_mutable_instance().i2cOpen(
			"/dev/i2c-1");

	// set virtual pin count address
	recvbuffer[0] = (VIRTUAL_IO_COUNT >> 8); //first high
	recvbuffer[1] = (VIRTUAL_IO_COUNT & 0xff); //second low

	// read data from i2c bus
	if (boost::serialization::singleton<I2C_Comm>::get_mutable_instance().Read_I2C_Bytes(
			getSlaveAddr(), recvbuffer, _16bit, 2)) {
		pincount = recvbuffer[0];
		modbus_INFO_WRITE("IOBoard[0x%x] pincount:%i", getSlaveAddr(), pincount);
	} else {
		modbus_ERROR_WRITE("IOBoard[0x%x] unable read pincount", getSlaveAddr());
		return false;
	}

	///add handler
	shared_ptr<MultiByteHandler> Multi;
	shared_ptr<SingleRegisterHandler> Single;
	shared_ptr<VersionHandler> Version;
	shared_ptr<HolRegHandler> Holding;
	shared_ptr<HolRegHandlerRO> HoldingRO;

	lock_guard<mutex> lock(*(boost::serialization::singleton<HandlerList>::get_mutable_instance().m_handlerlist_lock.get()));
	std::list<shared_ptr<MB_Framework::MBHandlerInt> > *phandlerlist = &(boost::serialization::singleton<
			HandlerList>::get_mutable_instance().m_handlerlist);

	std::list<shared_ptr<MB_Framework::MBHandlerInt> >::iterator handler_it = phandlerlist->begin(); // get first handler

	while (handler_it != phandlerlist->end()) {
	  shared_ptr<MB_Framework::MBHandlerInt> listitem =*handler_it;
		/* MultiByte */
		if (Multi.get() == nullptr ) {
			Multi = dynamic_pointer_cast<MultiByteHandler>(listitem);
		}
		/* SingleRegister */
		if (Single.get() == nullptr) {
			Single = dynamic_pointer_cast<SingleRegisterHandler>(listitem);
		}

		/* HolRegHandler */
		if (Holding.get() == nullptr ) {
			Holding = dynamic_pointer_cast<HolRegHandler>(listitem);
		}

		/* HolRegHandlerRO */
		if (HoldingRO.get() == nullptr ) {
			HoldingRO = dynamic_pointer_cast<HolRegHandlerRO>(listitem);
		}

		/* VersionHandler */
		if (Version.get() == nullptr ) {
			Version = dynamic_pointer_cast<VersionHandler>(listitem);
		}

		++handler_it;
	}

	/*
	 * create new specialist handler if not already in list
	 */

	if (Multi.get() == nullptr) {
		Multi = shared_ptr<MultiByteHandler>(new MultiByteHandler());
		phandlerlist->push_back(Multi);
	}

	if (Single.get() == nullptr) {
		Single = shared_ptr<SingleRegisterHandler>(new SingleRegisterHandler());
		phandlerlist->push_back(Single);
	}

	if (Holding.get() == nullptr) {
		Holding = shared_ptr<HolRegHandler>(new HolRegHandler(_16bit)); //virtual IO Port handler
		phandlerlist->push_back(Holding);
	}

	if (HoldingRO.get() == nullptr) {
	  shared_ptr<HolRegHandlerRO> tmp(new HolRegHandlerRO(_16bit));
		HoldingRO = tmp;
		phandlerlist->push_back(HoldingRO);
	}

	if (Version.get() == nullptr) {
		Version = shared_ptr<VersionHandler>(new VersionHandler());
		phandlerlist->push_back(Version);
	}

		TmpData.reset(new DataHandler());
		TmpData->setRange((VIRTUAL_DATA_START / 2),
				(VIRTUAL_DATA_START / 2) + (pincount * 2) - 1);

		PermData.reset(new DataHandler());
		PermData->setRange(((I2C_BUFFER_SIZE + EEPROM_DATA_START) / 2),
				((I2C_BUFFER_SIZE + EEPROM_DATA_START) / 2) + (pincount * 2) - 1);

	/**
	 * create mapping
	 */

	m_holding_handlerlist[0] = HoldingRO; /// SlaveID
	m_holding_handlerlist[1] = HoldingRO; /// EEPROM Write

	m_holding_handlerlist[VERSION_START / 2] = Version; /// Version

	m_holding_handlerlist[VIRTUAL_IO_COUNT/2] = HoldingRO; /// virtual pin count

	m_holding_handlerlist[VIRTUAL_IO_START/2] = Holding; /// virtual IO Port handler
	m_holding_handlerlist[(VIRTUAL_IO_START/2)+1] = Holding; /// virtual IO Port handler

	/// add all tmp data handler
	for (i = 0; i < pincount; i++) {
		handler_address = (VIRTUAL_DATA_START / 2) + (i * 2);
		m_holding_handlerlist[handler_address] = TmpData;
		modbus_DEBUG_WRITE("add TmpData handler[0x%x]", handler_address);
	}

	/// eeprom
	m_holding_handlerlist[I2C_BUFFER_SIZE / 2] = Holding; /// I2C Address

	/// add all perm data handler
	for (i = 0; i < pincount; i++) {
		handler_address = ((I2C_BUFFER_SIZE + EEPROM_DATA_START) / 2) + (i * 2);
		m_holding_handlerlist[handler_address] = PermData;
		modbus_DEBUG_WRITE("add PermData handler[0x%x]", handler_address);
	}

	/// add all function handler
	for (i = 0; i < pincount; i++) {
		handler_address = (I2C_BUFFER_SIZE / 2) + (EEPROM_FUNC_START / 2) + i;
		m_holding_handlerlist[handler_address] = Holding; /// add all function
		modbus_DEBUG_WRITE("add function handler[0x%x]", handler_address);
	}

	/// add all name handler
	for (i = 0; i < pincount; i++) {
		for (e = 0; e < IO_BOARD_MAX_IO_PIN_NAME_LENGTH / 2; e++) {
			handler_address = ((I2C_BUFFER_SIZE + EEPROM_NAME_START) / 2)
					+ (i * (IO_BOARD_MAX_IO_PIN_NAME_LENGTH / 2) + e);
			m_holding_handlerlist[handler_address] = Holding;
			modbus_DEBUG_WRITE("add name[%i] handler[0x%x]", i, handler_address);
		}
	}

	modbus_DEBUG_WRITE("finished");
	return true;
}

void IOBoard_Slave::getSlaveInfo(void) {
	uint8_t pincount, i;
	uint16_t i2c_address;
	/*
	 * address register (m_mode byte)
	 * data register (byte_count byte)
	 */
	uint8_t recvbuffer[_16bit + 0x20];

	modbus_INFO_WRITE("getSlaveInfo");

	/*
	 * get some informations directly from ioboard
	 */

	boost::serialization::singleton<I2C_Comm>::get_mutable_instance().i2cOpen("/dev/i2c-1");

	/* get SlaveID */
	recvbuffer[0] = 0; //first high
	recvbuffer[1] = 0; //second low
	// read data from i2c bus
	if (boost::serialization::singleton<I2C_Comm>::get_mutable_instance().Read_I2C_Bytes(
			getSlaveAddr(), recvbuffer, _16bit, 2)) {
		modbus_INFO_WRITE("IOBoard[0x%x]SlaveID:0x%x", getSlaveAddr(), recvbuffer[0]);

		if(m_mapping){
			memcpy(m_mapping->tab_registers,recvbuffer,2);	//store in DB
		}
	} else {
		modbus_ERROR_WRITE("IOBoard[0x%x] unable read SlaveID", getSlaveAddr());
	}

	/* get Version */
	i2c_address = VERSION_START;
	recvbuffer[0] = (i2c_address >> 8); //first high
	recvbuffer[1] = (i2c_address & 0xff); //second low
	// read data from i2c bus
	if (boost::serialization::singleton<I2C_Comm>::get_mutable_instance().Read_I2C_Bytes(
			getSlaveAddr(), recvbuffer, _16bit, VERSION_LENGTH)) {
		recvbuffer[VERSION_LENGTH] = '\0';

		if(m_mapping){
			memcpy(&m_mapping->tab_registers[VERSION_START/2],recvbuffer,VERSION_LENGTH);	//store in DB
			modbus_INFO_WRITE("IOBoard[0x%x]Version:%s", getSlaveAddr(), &m_mapping->tab_registers[VERSION_START/2]);
		}
	} else {
		modbus_ERROR_WRITE("IOBoard[0x%x] unable read version", getSlaveAddr());
	}

	/* get virtual pin count */
	i2c_address = VIRTUAL_IO_COUNT;
	recvbuffer[0] = (i2c_address >> 8); //first high
	recvbuffer[1] = (i2c_address & 0xff); //second low
	// read data from i2c bus
	if (boost::serialization::singleton<I2C_Comm>::get_mutable_instance().Read_I2C_Bytes(
			getSlaveAddr(), recvbuffer, _16bit, 2)) {
		pincount = recvbuffer[0];
		if(m_mapping){
			memcpy(&m_mapping->tab_registers[VIRTUAL_IO_COUNT/2],recvbuffer,2);	//store in DB
			modbus_INFO_WRITE("IOBoard[0x%x] pincount:%i", getSlaveAddr(), m_mapping->tab_registers[VIRTUAL_IO_COUNT/2]);
		}
	} else {
		modbus_ERROR_WRITE("IOBoard[0x%x] unable read pincount", getSlaveAddr());
	}

	/* get virtual pin functions */
	i2c_address = ((I2C_BUFFER_SIZE) + (EEPROM_FUNC_START));
	recvbuffer[0] = ( i2c_address >> 8); //first high
	recvbuffer[1] = ( i2c_address & 0xff); //second low
	modbus_DEBUG_WRITE("read function codes[0x%x]", i2c_address);
	// read data from i2c bus
	if (boost::serialization::singleton<I2C_Comm>::get_mutable_instance().Read_I2C_Bytes(
			getSlaveAddr(), recvbuffer, _16bit, pincount * 2)) {

		if(m_mapping){	// write values into DB
			memcpy(&m_mapping->tab_registers[i2c_address/2],recvbuffer,pincount * 2);	//store in DB
			for (i = 0; i < pincount; i++) {
				modbus_INFO_WRITE("function[%i]:0x%x", (i2c_address/2)+(i*2), m_mapping->tab_registers[(i2c_address/2)+(i*2)]);
			}
		}
	} else {
		modbus_DEBUG_WRITE("IOBoard[0x%x] unable read func codes", getSlaveAddr());
		modbus_ERROR_WRITE("IOBoard[0x%x] unable read func codes", getSlaveAddr());
	}

	/* get virtual pin names */
	for (i = 0; i < pincount; i++) {
		i2c_address = (I2C_BUFFER_SIZE) + (EEPROM_NAME_START) + (i * IO_BOARD_MAX_IO_PIN_NAME_LENGTH);
		// get virtual pin names
		recvbuffer[0] = (i2c_address >> 8); //first high
		recvbuffer[1] = (i2c_address & 0xff); //second low
		modbus_DEBUG_WRITE("name[%i]:0x%x", i, i2c_address);
		// read data from i2c bus
		if (boost::serialization::singleton<I2C_Comm>::get_mutable_instance().Read_I2C_Bytes(
				getSlaveAddr(), recvbuffer, _16bit, IO_BOARD_MAX_IO_PIN_NAME_LENGTH)) {
			recvbuffer[IO_BOARD_MAX_IO_PIN_NAME_LENGTH] = '\0';

			if(m_mapping){
				memcpy(&m_mapping->tab_registers[i2c_address/2],recvbuffer,IO_BOARD_MAX_IO_PIN_NAME_LENGTH);	//store in DB
				modbus_INFO_WRITE("name[%i]:%s", i, recvbuffer);
			}
		} else {
			modbus_DEBUG_WRITE("IOBoard[0x%x] unable read names", getSlaveAddr());
			modbus_ERROR_WRITE("IOBoard[0x%x] unable read names", getSlaveAddr());
		}//if(I2C)
	}//for(names)
}

} /* namespace I2C */
}/* namespace MB_Gateway */
}/* namespace icke2063 */
