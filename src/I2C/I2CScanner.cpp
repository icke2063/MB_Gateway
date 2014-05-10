/**
 * @file   I2CScanner.cpp
 * @Author icke
 * @date   02.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include <string.h>

#include "boost/serialization/singleton.hpp"

#include <I2CScanner.h>
#include <I2CComm.h>
#include <IOBoardSlave.h>
#include <SlaveList.h>

namespace icke2063 {
namespace MB_Gateway {
namespace I2C {

I2C_Scanner::I2C_Scanner(unsigned int timeout) :
		m_running(true),
		m_timeout(timeout) {
	/**
	 * Init Logging
	 * - set category name
	 * - connect with console
	 * - set loglevel
	 * @todo do this by configfile
	 */
	logger = &log4cpp::Category::getInstance(std::string("I2C_Scanner"));
	logger->setPriority(log4cpp::Priority::DEBUG);
	//if (console)logger->addAppender(console);

	m_scanner_thread.reset(new thread(&I2C_Scanner::thread_function, this)); // create new scheduler thread

	logger->info("I2C_Scanner");
	logger->debug("m_timeout: %d",m_timeout);
}

I2C_Scanner::~I2C_Scanner() {
	logger->info("enter ~I2C_Scanner");
	m_running = false;
	m_Condition.notify_all();
	if(m_scanner_thread.get() && m_scanner_thread->joinable())m_scanner_thread->join();
	logger->info("leave ~I2C_Scanner");
}

void I2C_Scanner::thread_function(void) {


	logger->info("I2C_Scanner Thread");
	/**
	 * list of counters
	 * - every slave has its own counter
	 * - if a slave is already listed but not found on I2C scna the counter will be increased
	 * - if maximum of retries is reached the slave will be removed from list
	 * - if slave is found again -> reset counter
	 */
	uint8_t slave_not_found_counter[MAX_I2C_SLAVE_ADR];
	memset(slave_not_found_counter, 0, MAX_I2C_SLAVE_ADR);	//initiate array

	shared_ptr<MBVirtualRTUSlave> curSlave;

	while (m_running) {
//		m_scanner_thread->yield();	
		unique_lock<mutex> lock(m_Mutex);

		if (m_Condition.wait_for(lock,
				chrono::milliseconds(m_timeout)) == cv_status::timeout) {	// wait a little bit

			I2C::I2C_Comm *i2cbus = &(boost::serialization::singleton<
					I2C::I2C_Comm>::get_mutable_instance());
			bool* p_livelist;
			int slaveID = 0;
			logger->debug("scan I2C bus...");
			if (i2cbus->i2cOpen(std::string("/dev/i2c-1"))) {//open or used already open i2c bus
				logger->debug("I2C bus open");
				if (((p_livelist = i2cbus->ScanBus())) != NULL) {//get livelist
					logger->debug("I2C scan finished");

					for (slaveID = 0; slaveID < MAX_I2C_SLAVE_ADR; slaveID++) {//check livelist

						curSlave =
								boost::serialization::singleton<SlaveList>::get_mutable_instance().getSlave(
										slaveID);
						if (p_livelist[slaveID]) {//found slave
							logger->info("Slave[0x%x]: found", slaveID);
							{
								slave_not_found_counter[slaveID] = 0;
								if (curSlave == NULL) {//only add not already listed slaves
									boost::serialization::singleton<SlaveList>::get_mutable_instance().addSlave(
											findSlaveType(slaveID));
								}
							}
						} else {//slave not found

							if (curSlave
									&& slave_not_found_counter[slaveID]++
											> MAX_SCAN_FAIL) {
								//remove slave from list
								logger->info("remove Slave[%d] from list\n",
										slaveID);
								boost::serialization::singleton<SlaveList>::get_mutable_instance().removeSlave(
										slaveID);
							}
						}
					}
				} else
					logger->error("ScanBus failure");
			}else{
				logger->error("failed to open I2C bus");
			}

		} else {
			logger->info("exit I2C_Scanner Thread");
			return;
		}
		usleep(1);
	}
}

shared_ptr<I2C_Slave> I2C_Scanner::findSlaveType (uint8_t slaveaddress){
	uint8_t readbuffer[4];
	shared_ptr<I2C_Slave> curSlave;


	I2C::I2C_Comm *i2cbus = &(boost::serialization::singleton<
			I2C::I2C_Comm>::get_mutable_instance());

	{
		/* IOBoard */
		//try to read slave ID
		readbuffer[0] = 0;
		i2cbus->Read_I2C_Bytes(slaveaddress,readbuffer,1,1);	//8Bit

		if(readbuffer[0] == SLAVE_ID){
			readbuffer[0] = 0;
			readbuffer[1] = 0;

			i2cbus->Read_I2C_Bytes(slaveaddress,readbuffer,2,1);	//16Bit
			if(readbuffer[0] == SLAVE_ID){
				logger->info("found IOBoard@%i",slaveaddress);
				curSlave = shared_ptr<IOBoard_Slave>(new IOBoard_Slave(slaveaddress));
			}
		}
	}

	logger->info("use default I2C Slave@%i",slaveaddress);
	if(curSlave.get() == NULL)curSlave = shared_ptr<I2C_Slave>(new I2C_Slave(slaveaddress));	//default handling
	curSlave-> init();
	return curSlave;
}

}}} /* namespace icke2063::MB_Gateway::I2C */
