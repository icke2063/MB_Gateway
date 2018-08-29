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
#include <I2CComm.h>

#define SLAVE_TYPE_I2C	0x10

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

class I2C_Slave: public VirtualRTUSlave
{
public:
	I2C_Slave(uint8_t SlaveAddr, I2CSLAVE_H_NS::shared_ptr<icke2063::I2C::I2C_Comm> sp_i2c_comm);
	virtual ~I2C_Slave() {}
	virtual uint8_t getType(void) {
		return SLAVE_TYPE_I2C;
	}

	virtual bool init(void);

	I2CSLAVE_H_NS::shared_ptr<icke2063::I2C::I2C_Comm> getI2CComm(){return m_sp_i2c_comm;}

protected:
	I2CSLAVE_H_NS::shared_ptr<icke2063::I2C::I2C_Comm> m_sp_i2c_comm;
	I2CSLAVE_H_NS::shared_ptr<MultiByteHandler> Multi;

};

} /* namespace I2C */
} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* I2CSLAVE_H_ */
