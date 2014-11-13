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

#include <build_options.h>

#ifndef ICKE2063_CRUMBY_NO_CPP11
	#include <memory>
	#define I2CSLAVE_H_NS std
#else
	#include <boost/shared_ptr.hpp>
	#define I2CSLAVE_H_NS boost
#endif

#include <VirtualRTUSlave.h>
#include <MultiByteHandler.h>
#include <Logger.h>

#define SLAVE_TYPE_I2C	0x10

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

class I2C_Slave: public VirtualRTUSlave, public common_cpp::Logger {
public:
	I2C_Slave(uint8_t SlaveAddr) :
			VirtualRTUSlave(SlaveAddr) {
		logger = &log4cpp::Category::getInstance(std::string("I2C_Slave"));
		logger->setPriority(log4cpp::Priority::DEBUG);
		if (console)
			logger->addAppender(console);

		logger->info("I2C_Slave[0x%x]", getSlaveAddr());

		m_mapping = modbus_mapping_new(0, 0, 128, 0);
		logger->debug("m_mapping:%x", m_mapping);

		//init();
	}
	virtual ~I2C_Slave() {
	}
	virtual uint8_t getType(void) {
		return SLAVE_TYPE_I2C;
	}

	virtual bool init(void) {
		int i;
		logger->info("init");

		///add handler
		Multi.reset(new MultiByteHandler()); //virtual IO Port handler
		Multi->setRange(0, 127);

		/**
		 * create mapping
		 */
		// add all tmp data handler
		for (i = 0; i < 127; i++) {
			m_input_handlerlist[i] = Multi;
			m_holding_handlerlist[i] = Multi;
			logger->debug("add Multi handler[0x%x]", i);
		}

		logger->debug("finished");
		return true;
	}

protected:
	I2CSLAVE_H_NS::shared_ptr<MultiByteHandler> Multi;

};

} /* namespace I2C */
} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* I2CSLAVE_H_ */
