/**
 * @file   I2CSlave.h
 * @Author icke
 * @date   02.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef I2CSLAVE_H_
#define I2CSLAVE_H_

#include <VirtualRTUSlave.h>
#include <MultiByteHandler.h>

namespace MB_Gateway {
namespace I2C {

class I2C_Slave: public VirtualRTUSlave, public Logger {
public:
	I2C_Slave(uint8_t SlaveID) :
			VirtualRTUSlave(SlaveID) {
		logger = &log4cpp::Category::getInstance(std::string("IOBoard_Slave"));
		logger->setPriority(log4cpp::Priority::DEBUG);
		if (console)
			logger->addAppender(console);

		logger->info("IOBoard_Slave[0x%x]", getSlaveID());

		m_mapping = modbus_mapping_new(0, 0,
				(I2C_BUFFER_SIZE + EEPROM_SIZE / 2), 0);
		logger->debug("m_mapping:%x", m_mapping);

		init();
	}
	virtual ~I2C_Slave() {
	}
	virtual uint8_t getType(void) {
		return 0x10;
	}
protected:
	virtual bool init(void) {
		int i, e;
		logger->info("init");

		///add handler
		Multi.reset(new MultiByteHandler()); //virtual IO Port handler
		Multi->setRange(0, 128);

		/**
		 * create mapping
		 */
		// add all tmp data handler
		for (i = 0; i < 128; i++) {
			m_handlerlist[i] = Multi.get();
			logger->debug("add Multi handler[0x%x]", i);
		}

		logger->debug("finished");
		return true;
	}

	auto_ptr<MultiByteHandler> Multi;

};

} /* namespace I2C */
} /* namespace MB_Gateway */
#endif /* I2CSLAVE_H_ */
