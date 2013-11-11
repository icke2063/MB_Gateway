/**
 * @file   SummerySlave.cpp
 * @Author icke
 * @date   03.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include "SummerySlave.h"
#include <handler/MultiRegisterHandler.h>
#include <handler/SRegisterHandler.h>

#include "boost/serialization/singleton.hpp"
#include <HandlerList.h>
#include <SlaveList.h>

namespace icke2063 {
namespace MB_Gateway {

SummerySlave::SummerySlave(uint8_t SlaveAddr, unsigned int timeout) :
		MB_Gateway::VirtualRTUSlave(SlaveAddr),m_running(true),
		p_scanner_thread(NULL), m_timeout(timeout){
	logger = &log4cpp::Category::getInstance(std::string("SummerySlave"));
	logger->setPriority(log4cpp::Priority::DEBUG);
	//if (console)logger->addAppender(console);

	logger->info("SummerySlave@%i",SlaveAddr);
	logger->debug("m_timeout: %d",m_timeout);

	init();

	p_scanner_thread.reset(new boost::thread(&SummerySlave::thread_function, this)); // create new scheduler thread

}

SummerySlave::~SummerySlave() {
	logger->info("SummerySlave");
	m_running = false;
	m_Condition.notify_all();
	if(p_scanner_thread.get() && p_scanner_thread->joinable())p_scanner_thread->join();
}

void SummerySlave::thread_function(void) {
	uint8_t slave;

	MBVirtualRTUSlave *curSlave;
	logger->debug("Summery Thread");

	while (m_running) {
		p_scanner_thread->yield();
		boost::mutex::scoped_lock lock(m_Mutex);

		if (!m_Condition.timed_wait(lock,
				boost::posix_time::milliseconds(m_timeout))) {
			logger->debug("scan slaves...");
			logger->debug("slavelist size: %d",boost::serialization::singleton<SlaveList>::get_mutable_instance().getList()->size());

			slave=0;
			/* loop over all detected slaves and insert data into list */
			do{
				curSlave =
						boost::serialization::singleton<SlaveList>::get_mutable_instance().getSlave(
								slave);
				if (curSlave) {
					m_mapping->tab_input_registers[slave] = curSlave->getType();
				} else {
					m_mapping->tab_input_registers[slave] = DEFAULT_SUMMERY_VALUE;

				}
				slave++;
			}while(slave<255);

			logger->debug("scan finished");

		} else {
			logger->debug("exit thread");
			return;
		}
		usleep(1);
	}
}

bool SummerySlave::init(void) {
	int i;

	/*
	 * address register (m_mode byte)
	 * data register (byte_count byte)
	 */

	logger->info("init");

	m_mapping = modbus_mapping_new(0, 0, 0, DEFAULT_SUMMERY_COUNT);

	///add handler
	MultiRegisterHandler *Multi = NULL;
	SRegisterHandler *Single = NULL;

	/*
	 * create new specialist handler if not already in list
	 */

	if (Multi == NULL) {
		Multi = new MultiRegisterHandler(m_mapping); //virtual IO Port handler
	}

	if (Single == NULL) {
		Single = new SRegisterHandler(m_mapping); //virtual IO Port handler
	}

	/**
	 * create mapping
	 */

	/// add all tmp data handler
	for (i = 0; i < DEFAULT_SUMMERY_COUNT; i++) {
		m_input_handlerlist[i] = Single;
	}

	logger->debug("init finished");
	return true;
}
} /* namespace MB_Gateway */
} /* namespace icke2063 */
