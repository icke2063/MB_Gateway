/**
 * @file   SummerySlave.cpp
 * @Author icke
 * @date   03.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include <sys/time.h>

#include "SummerySlave.h"
#include <handler/MultiRegisterHandler.h>
#include <handler/SRegisterHandler.h>

#include "boost/serialization/singleton.hpp"
#include <HandlerList.h>
#include <SlaveList.h>

#ifndef ICKE2063_CRUMBY_NO_CPP11
using namespace std;
#else
using namespace boost;
#endif

#include <crumby_logging_macros.h>

namespace icke2063 {
namespace MB_Gateway {

SummerySlave::SummerySlave(shared_ptr<threadpool::ThreadPool> delayed_pool, uint8_t SlaveAddr, unsigned int timeout) :
		MB_Gateway::VirtualRTUSlave(SlaveAddr),m_running(true), m_timeout(timeout),m_delayed_pool(delayed_pool){

	
	crumby_INFO_WRITE("SummerySlave@%i",SlaveAddr);
	crumby_DEBUG_WRITE("m_timeout: %d",m_timeout);

	crumby_DEBUG_WRITE("delayed_pool: 0x%x",delayed_pool.get());
	crumby_DEBUG_WRITE("m_delayed_pool: 0x%x",m_delayed_pool.get());
	

	init();

	//p_scanner_thread.reset(new std::thread(&SummerySlave::thread_function, this)); // create new scheduler thread
	
}

SummerySlave::~SummerySlave() {
	crumby_INFO_WRITE("SummerySlave");
	m_running = false;
	m_Condition.notify_all();
	//if(p_scanner_thread.get() && p_scanner_thread->joinable())p_scanner_thread->join();
}
void SummerySlave::startFunctor(void){
  crumby_INFO_WRITE("SummerySlave::startFunctor\n");
if(m_delayed_pool.get()){
    crumby_INFO_WRITE("addFunctor\n");
  struct timeval now;
  gettimeofday(&now,NULL);
  m_delayed_pool->delegateDelayedFunctor( shared_ptr<threadpool::DelayedFunctorInt>( new threadpool::DelayedFunctor( new SummerySlaveFunctor(shared_from_this()), &now) ));

}
  
}

void SummerySlave::SummerySlaveFunctor::functor_function(void) {
	uint8_t slave;

	if(!m_slave.get()){
		printf("SummerySlave::SummerySlaveFunctor::functor_function: m_slave failure\n");
	  return;
	}
	
	shared_ptr<MBVirtualRTUSlave> curSlave;
	crumby_DEBUG_WRITE("Summery Thread");

	if (m_slave->m_running) {
		//p_scanner_thread->yield();
		unique_lock<mutex> lock(m_slave->m_Mutex);

		if (m_slave->m_Condition.wait_for(lock,
				chrono::milliseconds(m_slave->m_timeout)) == cv_status::timeout ) {
			crumby_DEBUG_WRITE("scan slaves...");
			crumby_DEBUG_WRITE("slavelist size: %d",boost::serialization::singleton<SlaveList>::get_mutable_instance().getList()->size());

			slave=0;
			/* loop over all detected slaves and insert data into list */
			do{
				curSlave =
						boost::serialization::singleton<SlaveList>::get_mutable_instance().getSlave(
								slave);
				if (curSlave) {
					m_slave->m_mapping->tab_input_registers[slave] = curSlave->getType();
				} else {
					m_slave->m_mapping->tab_input_registers[slave] = DEFAULT_SUMMERY_VALUE;

				}
				slave++;
			}while(slave<255);

			crumby_DEBUG_WRITE("scan finished");
			
			/* some information about threadpool */
			if(m_slave->m_delayed_pool.get()){
			  m_slave->m_mapping->tab_input_registers[DEFAULT_SUMMERY_COUNT] = m_slave->m_delayed_pool->getLowWatermark();
			  m_slave->m_mapping->tab_input_registers[DEFAULT_SUMMERY_COUNT+1] = m_slave->m_delayed_pool->getHighWatermark();
			  m_slave->m_mapping->tab_input_registers[DEFAULT_SUMMERY_COUNT+2] = m_slave->m_delayed_pool->getWorkerCount();
			  m_slave->m_mapping->tab_input_registers[DEFAULT_SUMMERY_COUNT+3] = m_slave->m_delayed_pool->getQueueCount();
			  m_slave->m_mapping->tab_input_registers[DEFAULT_SUMMERY_COUNT+4] = m_slave->m_delayed_pool->getDQueueCount();
			}

		} else {
			crumby_DEBUG_WRITE("exit thread");
			return;
		}
		  struct timeval now;
		  gettimeofday(&now,NULL);
		  now.tv_usec += 100;

		 m_slave->m_delayed_pool->delegateDelayedFunctor(shared_ptr<threadpool::DelayedFunctorInt>(new threadpool::DelayedFunctor(new SummerySlaveFunctor(m_slave), &now)));


	}
}

bool SummerySlave::init(void) {
	int i;

	/*
	 * address register (m_mode byte)
	 * data register (byte_count byte)
	 */

	crumby_INFO_WRITE("init");

	m_mapping = modbus_mapping_new(0, 0, 0, DEFAULT_SUMMERY_COUNT+10);

	///add handler
	shared_ptr<MultiRegisterHandler> Multi;
	shared_ptr<SRegisterHandler> Single;

	/*
	 * create new specialist handler if not already in list
	 */

	if (Multi.get() == NULL) {
		Multi = shared_ptr<MultiRegisterHandler>(new MultiRegisterHandler(m_mapping)); //virtual IO Port handler
	}

	if (Single.get() == NULL) {
		Single = shared_ptr<SRegisterHandler>(new SRegisterHandler(m_mapping)); //virtual IO Port handler
	}

	/**
	 * create mapping
	 */

	/// add all tmp data handler
	for (i = 0; i < DEFAULT_SUMMERY_COUNT; i++) {
		m_input_handlerlist[i] = Single;
	}

	for(i = 0;i<5;i++){
	  m_input_handlerlist[DEFAULT_SUMMERY_COUNT+i] = Multi;
	}
	
	crumby_DEBUG_WRITE("init finished");
	return true;
}
} /* namespace MB_Gateway */
} /* namespace icke2063 */
