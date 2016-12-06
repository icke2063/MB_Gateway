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

#include "MBHandlerInt.h"
using namespace icke2063::MB_Framework;

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

#include <i2c_logging_macros.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

VersionHandler::VersionHandler(std::shared_ptr<I2C_Comm> sp_i2c_comm) :
		MultiByteHandler(sp_i2c_comm, _16bit, -1) {
	i2c_INFO_WRITE("VersionHandler");
	enableReadInpReg = false; //disable input register access
}

int VersionHandler::handleReadAccess(MB_Framework::MBHandlerParam *param) {

	uint8_t count = ((VERSION_LENGTH / 2) + (VERSION_LENGTH % 2));

	i2c_DEBUG_WRITE("VersionHandler::handleReadAccess");

	HandlerParam *curHandler = dynamic_cast<HandlerParam*>(param);
	if (curHandler != NULL) {
		/* check for the correct address range */
		setRange((VERSION_START / 2), ((VERSION_START / 2) + count)-1);
		if (!checkRange((curHandler->m_address), curHandler->m_count)){
			i2c_ERROR_WRITE("checkRange: failure");
			return 0;
		}
		if (curHandler->m_count != count){
			i2c_ERROR_WRITE("Count to low for correct version readout: %i of %i\n",curHandler->m_count,count);
			return 0;
		}
		return MultiByteHandler::handleReadAccess(param);
	}
	return 0; //return zero register handled > modbus exception
}

int DataHandler::handleReadAccess(MB_Framework::MBHandlerParam *param) {
	i2c_DEBUG_WRITE("DataHandler::handleReadAccess");

	HandlerParam *curHandler = dynamic_cast<HandlerParam*>(param);
	if (curHandler != NULL) {
		//check access alignment
		if (curHandler->m_address % 2 != 0) {
			i2c_ERROR_WRITE("address not correct aligned");
			return 0; //only even adresses -> alignment 2 register
		}
		if (curHandler->m_count % 2) {
			i2c_ERROR_WRITE("count not correct aligned");
			return 0; //only even counts -> alignement 2 register count
		}

		/* check for the correct address range */
		if (!checkRange((curHandler->m_address), curHandler->m_count)) {
			i2c_ERROR_WRITE("range failure");
			i2c_ERROR_WRITE("Range(0x%x,0x%x)", getRStart(), getRStop());
			i2c_ERROR_WRITE("checkRange(0x%x,0x%x)", curHandler->m_address,
					curHandler->m_count);
			return 0;
		}

		return MultiByteHandler::handleReadAccess(param);
	}
	return 0; //return zero register handled > modbus exception
}
int DataHandler::checkWriteAccess(MB_Framework::MBHandlerParam *param) {
	i2c_DEBUG_WRITE("DataHandler::checkWriteAccess");

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

IOBoard_Slave::IOBoard_Slave(uint8_t SlaveAddr, IOBOARDSLAVE_H_NS::shared_ptr<I2C::I2C_Comm> sp_i2c_comm) :
		I2C_Slave(SlaveAddr, sp_i2c_comm)
{

	i2c_INFO_WRITE("IOBoard_Slave[0x%x]", SlaveAddr);

	if(m_mapping)modbus_mapping_free(m_mapping);
	m_mapping = modbus_mapping_new(0, 0, (I2C_BUFFER_SIZE + EEPROM_SIZE / 2),
			0);
	i2c_DEBUG_WRITE("m_mapping:%x", m_mapping);

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

	i2c_INFO_WRITE("init");

	
	if(m_sp_i2c_comm.get() == NULL)
	{
	  i2c_ERROR_WRITE("IOBoard initialization failure: i2c comm invalid");
	  return false;
	  
	}
	
	/*
	 * get some informations directly from ioboard
	 */

	// set virtual pin count address
	recvbuffer[0] = (VIRTUAL_IO_COUNT >> 8); //first high
	recvbuffer[1] = (VIRTUAL_IO_COUNT & 0xff); //second low

	// read data from i2c bus
	if (m_sp_i2c_comm->Read_I2C_Bytes(
			getSlaveAddr(), recvbuffer, _16bit, 2)) {
		pincount = recvbuffer[0];
		i2c_INFO_WRITE("IOBoard[0x%x] pincount:%i", getSlaveAddr(), pincount);
	} else {
		i2c_ERROR_WRITE("IOBoard[0x%x] unable read pincount", getSlaveAddr());
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

	/** loop over complete Handlerlist and try to reuse already created ones */
	std::list<shared_ptr<MB_Framework::MBHandlerInt> >::iterator handler_it = phandlerlist->begin(); // get first handler
	while ( handler_it != phandlerlist->end() )
	{
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
		Multi = shared_ptr<MultiByteHandler>(new MultiByteHandler(m_sp_i2c_comm));
		phandlerlist->push_back(Multi);
	}

	if (Single.get() == nullptr) {
		Single = shared_ptr<SingleRegisterHandler>(new SingleRegisterHandler(m_sp_i2c_comm));
		phandlerlist->push_back(Single);
	}

	if (Holding.get() == nullptr) {
		Holding = shared_ptr<HolRegHandler>(new HolRegHandler(m_sp_i2c_comm, _16bit)); //virtual IO Port handler
		phandlerlist->push_back(Holding);
	}

	if (HoldingRO.get() == nullptr) {
	  shared_ptr<HolRegHandlerRO> tmp(new HolRegHandlerRO(m_sp_i2c_comm, _16bit));
		HoldingRO = tmp;
		phandlerlist->push_back(HoldingRO);
	}

	if (Version.get() == nullptr) {
		Version = shared_ptr<VersionHandler>(new VersionHandler(m_sp_i2c_comm));
		phandlerlist->push_back(Version);
	}

		TmpData.reset(new DataHandler(m_sp_i2c_comm));
		TmpData->setRange((VIRTUAL_DATA_START / 2),
				(VIRTUAL_DATA_START / 2) + (pincount * (VIRTUAL_DATA_LENGTH/2)) - 1);

		PermData.reset(new DataHandler(m_sp_i2c_comm));
		PermData->setRange(((I2C_BUFFER_SIZE + EEPROM_DATA_START) / 2),
				((I2C_BUFFER_SIZE + EEPROM_DATA_START) / 2) + (pincount * 2) - 1);

	/**
	 * create mapping
	 */
	uint16_t startaddr = 0;
	uint16_t endaddr = 0;

	/** RAM */
	m_holding_handlerlist[0] = HoldingRO; /// SlaveID
	m_holding_handlerlist[1] = HoldingRO; /// EEPROM Write

	m_holding_handlerlist[VERSION_START / 2] = Version; /// Version

	m_holding_handlerlist[VIRTUAL_IO_COUNT/2] = HoldingRO; /// virtual pin count

	m_holding_handlerlist[VIRTUAL_IO_START/2] = Holding; /// virtual IO Port handler
	m_holding_handlerlist[(VIRTUAL_IO_START/2)+1] = Holding; /// virtual IO Port handler

	/// add all tmp data handler
	{
	    startaddr = (VIRTUAL_DATA_START / 2);
	    endaddr = startaddr + ((pincount*(VIRTUAL_DATA_LENGTH/2)) - 1);
	    std::shared_ptr<MBBlockInt> funcblock(new MBBlockInt( TmpData, startaddr, endaddr ));

	    m_holding_blocklist.push_back( funcblock );
	    i2c_DEBUG_WRITE("add Tmpdata handlerblock[%i..%i]", funcblock->getStartAddr(), funcblock->getEndAddr());
	}

	/** eeprom */
	m_holding_handlerlist[I2C_BUFFER_SIZE / 2] = Holding; /// I2C Address

	/// add all function handler
	{
	    startaddr = ( I2C_BUFFER_SIZE / 2 ) + ( EEPROM_FUNC_START / 2 );
	    endaddr = startaddr + pincount - 1;
	    std::shared_ptr<MBBlockInt> funcblock(new MBBlockInt( Holding, startaddr, endaddr ));

	    m_holding_blocklist.push_back( funcblock );
	    i2c_DEBUG_WRITE("add function handlerblock[%i..%i]", funcblock->getStartAddr(), funcblock->getEndAddr());
	}
	
	/// add all name handler
	{
	    startaddr = ((I2C_BUFFER_SIZE + EEPROM_NAME_START) / 2);
	    endaddr = startaddr + (pincount * (IO_BOARD_MAX_IO_PIN_NAME_LENGTH / 2)) - 1;
	    std::shared_ptr<MBBlockInt> funcblock(new MBBlockInt( Holding, startaddr, endaddr ));

	    m_holding_blocklist.push_back( funcblock );
	    i2c_DEBUG_WRITE("add name handlerblock[%i..%i]", funcblock->getStartAddr(), funcblock->getEndAddr());
	}

	/// add all perm data handler
	{
	    startaddr = ((I2C_BUFFER_SIZE + EEPROM_DATA_START) / 2);
	    endaddr = startaddr + (pincount * (EEPROM_DATA_LENGTH / 2)) - 1;
	    std::shared_ptr<MBBlockInt> funcblock(new MBBlockInt( PermData, startaddr, endaddr ));

	    m_holding_blocklist.push_back( funcblock );
	    i2c_DEBUG_WRITE("add PermData handlerblock[%i..%i]", funcblock->getStartAddr(), funcblock->getEndAddr());
	}

	{
	    startaddr = ((I2C_BUFFER_SIZE + EEPROM_SHARED_DATA_START) / 2);
	    endaddr = startaddr + EEPROM_SHARED_DATA_LENGTH - 1;
	    std::shared_ptr<MBBlockInt> funcblock(new MBBlockInt( Holding, startaddr, endaddr ));

	    m_holding_blocklist.push_back( funcblock );
	    i2c_DEBUG_WRITE("add SharedData handlerblock[%i..%i]", funcblock->getStartAddr(), funcblock->getEndAddr());
	}



	i2c_DEBUG_WRITE("finished");
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

	i2c_INFO_WRITE("getSlaveInfo");

	/*
	 * get some informations directly from ioboard
	 */
	if(m_sp_i2c_comm.get() == NULL
			|| m_sp_i2c_comm->i2cOpen() == false)
	{
		i2c_ERROR_WRITE("IOBoard I2C open error\n");
		return;
	}

	/* get SlaveID */
	recvbuffer[0] = 0; //first high
	recvbuffer[1] = 0; //second low
	// read data from i2c bus
	if (m_sp_i2c_comm->Read_I2C_Bytes(
			getSlaveAddr(), recvbuffer, _16bit, 2)) {
		i2c_INFO_WRITE("IOBoard[0x%x]SlaveID:0x%x", getSlaveAddr(), recvbuffer[0]);

		if(m_mapping){
			memcpy(m_mapping->tab_registers,recvbuffer,2);	//store in DB
		}
	} else {
		i2c_ERROR_WRITE("IOBoard[0x%x] unable read SlaveID", getSlaveAddr());
	}

	/* get Version */
	i2c_address = VERSION_START;
	recvbuffer[0] = (i2c_address >> 8); //first high
	recvbuffer[1] = (i2c_address & 0xff); //second low
	// read data from i2c bus
	if (m_sp_i2c_comm->Read_I2C_Bytes(
			getSlaveAddr(), recvbuffer, _16bit, VERSION_LENGTH)) {
		recvbuffer[VERSION_LENGTH] = '\0';

		if(m_mapping){
			memcpy(&m_mapping->tab_registers[VERSION_START/2],recvbuffer,VERSION_LENGTH);	//store in DB
			i2c_INFO_WRITE("IOBoard[0x%x]Version:%s", getSlaveAddr(), &m_mapping->tab_registers[VERSION_START/2]);
		}
	} else {
		i2c_ERROR_WRITE("IOBoard[0x%x] unable read version", getSlaveAddr());
	}

	/* get virtual pin count */
	i2c_address = VIRTUAL_IO_COUNT;
	recvbuffer[0] = (i2c_address >> 8); //first high
	recvbuffer[1] = (i2c_address & 0xff); //second low
	// read data from i2c bus
	if (m_sp_i2c_comm->Read_I2C_Bytes(
			getSlaveAddr(), recvbuffer, _16bit, 2)) {
		pincount = recvbuffer[0];
		if(m_mapping){
			memcpy(&m_mapping->tab_registers[VIRTUAL_IO_COUNT/2],recvbuffer,2);	//store in DB
			i2c_INFO_WRITE("IOBoard[0x%x] pincount:%i", getSlaveAddr(), m_mapping->tab_registers[VIRTUAL_IO_COUNT/2]);
		}
	} else {
		i2c_ERROR_WRITE("IOBoard[0x%x] unable read pincount", getSlaveAddr());
	}

	/* get virtual pin functions */
	i2c_address = ((I2C_BUFFER_SIZE) + (EEPROM_FUNC_START));
	recvbuffer[0] = ( i2c_address >> 8); //first high
	recvbuffer[1] = ( i2c_address & 0xff); //second low
	i2c_DEBUG_WRITE("read function codes[0x%x]", i2c_address);
	// read data from i2c bus
	if (m_sp_i2c_comm->Read_I2C_Bytes(
			getSlaveAddr(), recvbuffer, _16bit, pincount * 2)) {

		if(m_mapping){	// write values into DB
			memcpy(&m_mapping->tab_registers[i2c_address/2],recvbuffer,pincount * 2);	//store in DB
			for (i = 0; i < pincount; i++) {
				i2c_INFO_WRITE("function[%i]:0x%x", (i2c_address/2)+(i*2), m_mapping->tab_registers[(i2c_address/2)+(i*2)]);
			}
		}
	} else {
		i2c_DEBUG_WRITE("IOBoard[0x%x] unable read func codes", getSlaveAddr());
		i2c_ERROR_WRITE("IOBoard[0x%x] unable read func codes", getSlaveAddr());
	}

	/* get virtual pin names */
	for (i = 0; i < pincount; i++) {
		i2c_address = (I2C_BUFFER_SIZE) + (EEPROM_NAME_START) + (i * IO_BOARD_MAX_IO_PIN_NAME_LENGTH);
		// get virtual pin names
		recvbuffer[0] = (i2c_address >> 8); //first high
		recvbuffer[1] = (i2c_address & 0xff); //second low
		i2c_DEBUG_WRITE("name[%i]:0x%x", i, i2c_address);
		// read data from i2c bus
		if (m_sp_i2c_comm->Read_I2C_Bytes(
				getSlaveAddr(), recvbuffer, _16bit, IO_BOARD_MAX_IO_PIN_NAME_LENGTH)) {
			recvbuffer[IO_BOARD_MAX_IO_PIN_NAME_LENGTH] = '\0';

			if(m_mapping){
				memcpy(&m_mapping->tab_registers[i2c_address/2],recvbuffer,IO_BOARD_MAX_IO_PIN_NAME_LENGTH);	//store in DB
				i2c_INFO_WRITE("name[%i]:%s", i, recvbuffer);
			}
		} else {
			i2c_DEBUG_WRITE("IOBoard[0x%x] unable read names", getSlaveAddr());
			i2c_ERROR_WRITE("IOBoard[0x%x] unable read names", getSlaveAddr());
		}//if(I2C)
	}//for(names)
}

} /* namespace I2C */
}/* namespace MB_Gateway */
}/* namespace icke2063 */
