/**
 * @file   IOBoardSlave.h
 * @Author icke
 * @date   02.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef IOBOARDSLAVE_H_
#define IOBOARDSLAVE_H_

#include <build_options.h>

#ifndef ICKE2063_CRUMBY_NO_CPP11
	#include <memory>
	#define IOBOARDSLAVE_H_NS std
#else
	#include <boost/shared_ptr.hpp>
	#define IOBOARDSLAVE_H_NS boost
#endif


#include "I2C_IO_BOARD.git/base/inc/slave_eeprom_mapping.h"

#define SLAVE_TYPE_IOBOARD	SLAVE_ID

#include <MultiByteHandler.h>

#include <I2CSlave.h>
#include <HandlerParam.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {


class VersionHandler: public MultiByteHandler {
public:
	VersionHandler(IOBOARDSLAVE_H_NS::shared_ptr<icke2063::I2C::I2C_Comm> sp_i2c_comm);
	virtual ~VersionHandler() {}

	virtual int handleReadAccess(MB_Framework::MBHandlerParam *param);
	virtual int checkWriteAccess(MB_Framework::MBHandlerParam *param) {
		return 0;
	}	//disable write access
	virtual int handleWriteAccess(MB_Framework::MBHandlerParam *param) {
		return 0;
	}	//disable write access
};

class DataHandler: public MultiByteHandler {
public:
	DataHandler(IOBOARDSLAVE_H_NS::shared_ptr<icke2063::I2C::I2C_Comm> sp_i2c_comm):
		MultiByteHandler(sp_i2c_comm, _16bit,-1){}
	virtual ~DataHandler() {}

	virtual int handleReadAccess(MB_Framework::MBHandlerParam *param);
	virtual int checkWriteAccess(MB_Framework::MBHandlerParam *param);
};


class IOBoard_Slave: public I2C_Slave {
public:
	IOBoard_Slave(uint8_t SlaveAddr,
			IOBOARDSLAVE_H_NS::shared_ptr<icke2063::I2C::I2C_Comm> sp_i2c_comm);
	virtual ~IOBoard_Slave();
	virtual bool init(void);

	/**
	 * read all (almost) static infos from I2C IO board and store it in DB
	 */
	void getSlaveInfo(void);
	virtual uint8_t getType( void ){return SLAVE_TYPE_IOBOARD;}

private:
	IOBOARDSLAVE_H_NS::shared_ptr<DataHandler> TmpData;
	IOBOARDSLAVE_H_NS::shared_ptr<DataHandler> PermData;
};

} /* namespace I2C */
}/* namespace MB_Gateway */
}/* namespace icke2063 */
#endif /* IOBOARDSLAVE_H_ */
