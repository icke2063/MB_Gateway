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

namespace MB_Gateway {

SummerySlave::SummerySlave(uint8_t ID) :
		MB_Gateway::VirtualRTUSlave(ID),m_running(true) {
	logger = &log4cpp::Category::getInstance(std::string("SummerySlave"));
	logger->setPriority(log4cpp::Priority::DEBUG);
	if (console)
		logger->addAppender(console);

	logger->info("SummerySlave");

	boost::thread t1(boost::bind(&SummerySlave::thread_function, this)); // create new scheduler thread
	p_scanner_thread = &t1; // save pointer of thread object

	init();

}

SummerySlave::~SummerySlave() {
	logger->info("SummerySlave");
	m_running = false;
	m_Condition.notify_all();
	p_scanner_thread->join();
}

void SummerySlave::thread_function(void) {
	uint8_t slaveID;

	MBVirtualRTUSlave *curSlave;

	while (m_running) {
		p_scanner_thread->yield();
		boost::mutex::scoped_lock lock(m_Mutex);

		if (!m_Condition.timed_wait(lock,
				boost::posix_time::milliseconds(m_timeout))) {

			/* loop over all detected slaves and insert data into list */
			for (slaveID = 0; slaveID < 256; slaveID++) {
				curSlave =
						boost::serialization::singleton<SlaveList>::get_mutable_instance().getSlave(
								slaveID);
				if (curSlave) {
					m_mapping->tab_input_registers[slaveID] =
							curSlave->getType();
				} else {
					m_mapping->tab_input_registers[slaveID] = 0;

				}
			}
		}
	}
}

bool SummerySlave::init(void) {
	int i;

	/*
	 * address register (m_mode byte)
	 * data register (byte_count byte)
	 */

	logger->info("init");

	m_mapping = modbus_mapping_new(0, 0, 256, 256);

	///add handler
	MultiRegisterHandler *Multi = NULL;
	SRegisterHandler *Single = NULL;

	boost::lock_guard<boost::mutex> lock(
			*(boost::serialization::singleton<HandlerList>::get_mutable_instance().p_handlerlist_lock->getMutex()));
	list<MBHandlerInt*> *phandlerlist = &(boost::serialization::singleton<
			HandlerList>::get_mutable_instance().m_handlerlist);

	list<MBHandlerInt*>::iterator handler_it = phandlerlist->begin(); // get first handler

	while (handler_it != phandlerlist->end()) {
		/* MultiByte */
		if (Multi == NULL) {
			Multi = dynamic_cast<MultiRegisterHandler*>(*handler_it);
		}
		/* SingleRegister */
		if (Single == NULL) {
			Single = dynamic_cast<SRegisterHandler*>(*handler_it);
		}

		++handler_it;
	}

	/*
	 * create new specialist handler if not already in list
	 */

	if (Multi == NULL) {
		Multi = new MultiRegisterHandler(m_mapping); //virtual IO Port handler
		phandlerlist->push_back(Multi);
	}

	if (Single == NULL) {
		Single = new SRegisterHandler(m_mapping); //virtual IO Port handler
	}

	/**
	 * create mapping
	 */

	/// add all tmp data handler
	for (i = 0; i < 255; i++) {
		m_handlerlist[i] = Single;
	}

	logger->debug("finished");
	return true;
}
} /* namespace MB_Gateway */
