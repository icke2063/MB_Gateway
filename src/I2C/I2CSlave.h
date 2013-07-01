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
namespace MB_Gateway {
namespace I2C {

class I2C_Slave:public VirtualRTUSlave {
public:
	I2C_Slave(uint8_t SlaveID):VirtualRTUSlave(SlaveID){};
	virtual ~I2C_Slave(){};
};

} /* namespace I2C */
} /* namespace MB_Gateway */
#endif /* I2CSLAVE_H_ */
