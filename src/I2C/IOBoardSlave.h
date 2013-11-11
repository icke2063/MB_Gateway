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

#include <auto_ptr.h>
using namespace std;



#include "I2C_IO_BOARD.git/base/inc/slave_eeprom_mapping.h"

#define SLAVE_TYPE_IOBOARD	SLAVE_ID

#include <Logger.h>
#include <MultiByteHandler.h>
using namespace icke2063::MB_Framework;

#include <I2CSlave.h>
#include <HandlerParam.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {


class VersionHandler: public MultiByteHandler {
public:
	VersionHandler();
	virtual ~VersionHandler() {}

	virtual int handleReadAccess(MBHandlerParam *param);
	virtual int checkWriteAccess(MBHandlerParam *param) {
		return 0;
	}	//disable write access
	virtual int handleWriteAccess(MBHandlerParam *param) {
		return 0;
	}	//disable write access
};

class DataHandler: public MultiByteHandler {
public:
	DataHandler():MultiByteHandler(_16bit,-1){}
	virtual ~DataHandler() {}

	virtual int handleReadAccess(MBHandlerParam *param);
	virtual int checkWriteAccess(MBHandlerParam *param);
};


class IOBoard_Slave: public I2C_Slave {
public:
	IOBoard_Slave(uint8_t SlaveAddr);
	virtual ~IOBoard_Slave();
	virtual bool init(void);

	/**
	 * read all (almost) static infos from I2C IO board and store it in DB
	 */
	void getSlaveInfo(void);
	virtual uint8_t getType( void ){return SLAVE_TYPE_IOBOARD;}

private:
	auto_ptr<DataHandler> TmpData;
	auto_ptr<DataHandler> PermData;
};

} /* namespace I2C */
}/* namespace MB_Gateway */
}/* namespace icke2063 */
#endif /* IOBOARDSLAVE_H_ */
