/**
 * @file   I2CScanner.cpp
 * @Author icke
 * @date   02.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include "boost/serialization/singleton.hpp"

#include "I2CScanner.h"
#include <I2CComm.h>
#include <IOBoardSlave.h>
#include <SlaveList.h>

namespace MB_Gateway {
namespace I2C {

I2C_Scanner::I2C_Scanner(unsigned int timeout) :
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
	if (console)
		logger->addAppender(console);

	logger->info("I2C_Scanner");

	boost::thread t1(boost::bind(&I2C_Scanner::thread_function, this)); // create new scheduler thread
	p_scanner_thread = &t1; // save pointer of thread object

}

I2C_Scanner::~I2C_Scanner() {
	m_running = false;
	m_Condition.notify_all();
	p_scanner_thread->join();
	logger->info("~WorkerThread");
}

void I2C_Scanner::thread_function(void) {

	uint8_t slave_not_found_counter[MAX_I2C_SLAVE_ADR];
	memset(slave_not_found_counter, 0, MAX_I2C_SLAVE_ADR);

	MBVirtualRTUSlave *curSlave;

	while (m_running) {
		p_scanner_thread->yield();

		boost::mutex::scoped_lock lock(m_Mutex);

		if (!m_Condition.timed_wait(lock,
				boost::posix_time::milliseconds(m_timeout))) {
			I2C::I2C_Comm *i2cbus = &(boost::serialization::singleton<
					I2C::I2C_Comm>::get_mutable_instance());
			bool* p_livelist;
			int slaveID = 0;
			logger->debug("scan I2C bus...");
			if (i2cbus->i2cOpen(std::string("/dev/i2c-1"))) {
				if (((p_livelist = i2cbus->ScanBus())) != NULL) {
					logger->debug("ScanBus done");

					for (slaveID = 0; slaveID < MAX_I2C_SLAVE_ADR; slaveID++) {

						curSlave =
								boost::serialization::singleton<SlaveList>::get_mutable_instance().getSlave(
										slaveID);
						if (p_livelist[slaveID]) {
							logger->debug("Slave[0x%x]: found", slaveID);
							{
								slave_not_found_counter[slaveID] = 0;
								if (curSlave == NULL) {
									boost::serialization::singleton<SlaveList>::get_mutable_instance().addSlave(
											slaveID,
											new I2C::IOBoard_Slave(slaveID));
								}
							}
						} else {

							if (curSlave
									&& slave_not_found_counter[slaveID]++
											> MAX_SCAN_FAIL) {
								//remove slave from list
								logger->info("remove Slave[%d] from list\n",
										slaveID);
								boost::serialization::singleton<SlaveList>::get_mutable_instance().deleteSlave(
										slaveID);
							}
						}
					}
				} else
					logger->error("ScanBus failure");
			}

		} else {
			return;
		}
	}
}

}
} /* namespace MB_Gateway::I2C */
