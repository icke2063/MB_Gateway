/**
 * @file   WorkerThread.cpp
 * @Author icke2063
 * @date   31.05.2013
 * @brief  MBWorkerThread implementation
 *
 * Copyright © 2013 icke2063 <icke2063@gmail.com>
 *
 * This framework is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "WorkerThread.h"

namespace MB_Gateway {

WorkerThread::WorkerThread(deque<MBFunctor *> *functor_queue,
		Mutex *functor_lock):
		p_functor_lock(functor_lock), p_functor_queue(functor_queue), m_running(
				true),curFunctor(NULL) {

	/**
	 * Init Logging
	 * - set category name
	 * - connect with console
	 * - set loglevel
	 * @todo do this by configfile
	 */
	logger = &log4cpp::Category::getInstance(std::string("WorkerThread"));
	logger->setPriority(log4cpp::Priority::DEBUG);
	if(console)logger->addAppender(console);

	logger->info("WorkerThread");

	boost::thread t1(boost::bind(&WorkerThread::thread_function, this));	// create new scheduler thread
	p_worker_thread = &t1; 													// save pointer of thread object
}

WorkerThread::~WorkerThread() {
	m_running = false;
	p_worker_thread->join();
	logger->info("~WorkerThread");

}

void WorkerThread::thread_function(void) {

	while (m_running) {
		p_worker_thread->yield();
		{
			boost::lock_guard<boost::mutex> lock(p_functor_lock->getMutex()); // lock before queue access

			if (p_functor_queue->size() > 0) {
				curFunctor.reset(p_functor_queue->front()); // get next functor from queue
				p_functor_queue->pop_front();				// remove functor from queue
			}
		}

		if(curFunctor.get() != NULL){
			logger->debug("get next functor");
			curFunctor->functor_function(); // call handling function
			curFunctor.reset(NULL); 		// reset pointer
		}
	}
}

} /* namespace MB_Framework */
