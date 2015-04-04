/**
 * @file   I2CScanner.cpp
 * @Author icke
 * @date   02.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include <build_options.h>

#include <string.h>

#include "boost/serialization/singleton.hpp"

#include <I2CScanner.h>
#include <SlaveList.h>

#ifdef CRUMBY_I2C_IOBOARD
  #include <IOBoardSlave.h>
#endif

#include <i2c_logging_macros.h>

using namespace I2CSCANNER_H_NS;

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

I2C_Scanner::I2C_Scanner(std::string& i2c_master_path, unsigned int timeout_ms) :
		m_running(true),
		m_timeout_ms(timeout_ms),
		m_I2C_master_path(i2c_master_path){

	m_scanner_thread.reset(new thread(&I2C_Scanner::thread_function, this)); // create new scheduler thread

	i2c_INFO_WRITE("I2C_Scanner\n");
	i2c_DEBUG_WRITE("m_timeout_ms: %d\n",m_timeout_ms);
	m_sp_i2cbus_comm.reset(new I2C::I2C_Comm(m_I2C_master_path));
}

I2C_Scanner::~I2C_Scanner() {
	i2c_INFO_WRITE("enter ~I2C_Scanner");
	m_running = false;
	m_Condition.notify_all();
	if(m_scanner_thread.get() && m_scanner_thread->joinable())m_scanner_thread->join();
	i2c_INFO_WRITE("leave ~I2C_Scanner");
}

void I2C_Scanner::thread_function(void) {


	i2c_INFO_WRITE("I2C_Scanner Thread");
	/**
	 * list of counters
	 * - every slave has its own counter
	 * - if a slave is already listed but not found on I2C scna the counter will be increased
	 * - if maximum of retries is reached the slave will be removed from list
	 * - if slave is found again -> reset counter
	 */
	uint8_t slave_not_found_counter[MAX_I2C_SLAVE_ADR];
	memset(slave_not_found_counter, 0, MAX_I2C_SLAVE_ADR);	//initiate array

	shared_ptr<MB_Framework::MBVirtualRTUSlave> curSlave;

	while (m_running) {
//		m_scanner_thread->yield();	
		unique_lock<mutex> lock(m_Mutex);

		if (m_Condition.wait_for(lock, chrono::milliseconds(m_timeout_ms)) == cv_status::timeout)
		{	// wait a little bit
			bool* p_livelist;
			int slaveID = 0;
			i2c_DEBUG_WRITE("scan I2C bus...");
			if (m_sp_i2cbus_comm.get() && m_sp_i2cbus_comm->i2cOpen())
			{//open or used already open i2c bus
				i2c_DEBUG_WRITE("I2C bus open");
				if (((p_livelist = m_sp_i2cbus_comm->ScanBus())) != NULL) 
				{//get livelist
					i2c_DEBUG_WRITE("I2C scan finished");

					for (slaveID = 0; slaveID < MAX_I2C_SLAVE_ADR; slaveID++)
					{//check livelist
//						i2c_DEBUG_WRITE("check I2C Slave[%u]\n", slaveID);

						curSlave = boost::serialization::singleton<SlaveList>::get_mutable_instance().getSlave(slaveID);
						
						if (p_livelist[slaveID])
						{//found slave
							i2c_INFO_WRITE("I2C Slave[%u]: found", slaveID);
							{
								slave_not_found_counter[slaveID] = 0;
								if (curSlave.get() == NULL) 
								{//only add not already listed slaves
									boost::serialization::singleton<SlaveList>::get_mutable_instance().addSlave(
											createSlavebyType(slaveID));
								}
							}
						} else {//slave not found

							if ((curSlave.get() != NULL)
								&& slave_not_found_counter[slaveID]++ > MAX_SCAN_FAIL)
							{
								//remove slave from list
								i2c_INFO_WRITE("remove I2C Slave[%u] from list", slaveID);
								boost::serialization::singleton<SlaveList>::get_mutable_instance().removeSlave(slaveID);
							}
						}
					}
				} 
				else
				{  
					i2c_ERROR_WRITE("ScanBus failure");
				}	  
			}
			else
			{
				i2c_ERROR_WRITE("failed to open I2C bus");
			}

		} else {
			i2c_INFO_WRITE("exit I2C_Scanner Thread");
			return;
		}
		usleep(1);
	}
}

shared_ptr<I2C_Slave> I2C_Scanner::createSlavebyType (uint8_t slaveaddress)
{
	uint8_t readbuffer[4];
	shared_ptr<I2C_Slave> curSlave;

	if(m_sp_i2cbus_comm.get())
	{
#ifdef CRUMBY_I2C_IOBOARD
		/* IOBoard */
		//try to read slave ID
		readbuffer[0] = 0;
		m_sp_i2cbus_comm->Read_I2C_Bytes(slaveaddress,readbuffer,1,1);	//8Bit

		if(readbuffer[0] == SLAVE_ID){
			readbuffer[0] = 0;
			readbuffer[1] = 0;

			m_sp_i2cbus_comm->Read_I2C_Bytes(slaveaddress,readbuffer,2,1);	//16Bit
			if(readbuffer[0] == SLAVE_ID){
				i2c_INFO_WRITE("found IOBoard@%i", slaveaddress);
				curSlave = shared_ptr<IOBoard_Slave>(new IOBoard_Slave(slaveaddress, m_sp_i2cbus_comm));
			}
		}
#endif

		if(curSlave.get() == NULL)
		{
			i2c_INFO_WRITE("use default I2C Slave@%i", slaveaddress);
			curSlave = shared_ptr<I2C_Slave>(new I2C_Slave(slaveaddress, m_sp_i2cbus_comm));	//default handling
		}
		  curSlave-> init();
	}
	return curSlave;
}

}}} /* namespace icke2063::MB_Gateway::I2C */
