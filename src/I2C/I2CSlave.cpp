/**
 * @file   I2CSlave.h
 * @Author icke
 * @date   02.06.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include <I2CSlave.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

I2C_Slave::I2C_Slave(uint8_t SlaveAddr, I2CSLAVE_H_NS::shared_ptr<I2C::I2C_Comm> sp_i2c_comm) :
		VirtualRTUSlave(SlaveAddr)
		,m_sp_i2c_comm(sp_i2c_comm)
{

	i2c_INFO_WRITE("I2C_Slave[0x%x]", getSlaveAddr());

	m_mapping = modbus_mapping_new(0, 0, 128, 0);
	i2c_DEBUG_WRITE("m_mapping:%x", m_mapping);

	//init();
}

bool I2C_Slave::init(void) {
		int i;
		i2c_INFO_WRITE("init");

		///add handler
		Multi.reset(new MultiByteHandler(m_sp_i2c_comm)); //virtual IO Port handler
		Multi->setRange(0, 127);

		/**
		 * create mapping
		 */
		// add all tmp data handler
		for (i = 0; i < 127; i++) {
			m_input_handlerlist[i] = Multi;
			m_holding_handlerlist[i] = Multi;
			i2c_DEBUG_WRITE("add Multi handler[0x%x]", i);
		}

		i2c_DEBUG_WRITE("finished");
		return true;
	}


} /* namespace I2C */
} /* namespace MB_Gateway */
} /* namespace icke2063 */
