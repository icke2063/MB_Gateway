/**
 * @file   ThreadPool.cpp
 * @Author icke
 * @date   28.05.2013
 * @brief  MBThreadPool implementation
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

#include <ThreadPool.h>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/time_duration.hpp>

#include <iostream>
#include <functional>
#include <vector>
#include <algorithm>
#include <string>

#include <WorkerThread.h>

namespace MB_Gateway {

ThreadPool::ThreadPool() :
		running(true) {
	/**
	 * Init Logging
	 * - set category name
	 * - connect with console
	 * - set loglevel
	 * @todo do this by configfile
	 */
	logger = &log4cpp::Category::getInstance(std::string("ThreadPool"));
	logger->setPriority(log4cpp::Priority::DEBUG);
	if (console)
		logger->addAppender(console);

	logger->info("ThreadPool");

	p_functor_lock = new Mutex(); //init mutex for functor list
	logger->debug("p_functor_lock:%x", p_functor_lock);
	boost::thread t1(boost::bind(&ThreadPool::scheduler, this)); // create new scheduler thread
	p_scheduler_thread = &t1; //save pointer of thread object

}

ThreadPool::~ThreadPool() {
	running = false;
	if (p_scheduler_thread)
		p_scheduler_thread->join();
	logger->info("~ThreadPool");
}

void ThreadPool::scheduler(void) {
	unsigned int max_func_size = 1;

	logger->info("ThreadPool scheduler thread");

	while (m_workerThreads.size() < this->getLowWatermark()) {
		logger->debug("autocreate new worker thread: %i of %i",
				m_workerThreads.size() + 1, this->getHighWatermark());
		logger->debug("p_functor_lock[%x] m_functor_queue[%x]", p_functor_lock,
				&m_functor_queue);
		WorkerThread *newWorker = new WorkerThread(&m_functor_queue,
				p_functor_lock);
		m_workerThreads.insert(newWorker);
	}

	while (running) {
		p_scheduler_thread->yield();

		// check functor list

		if (p_functor_lock != NULL
				&& ((Mutex*) (p_functor_lock))->getMutex() != NULL) {
			boost::lock_guard<boost::mutex> lock(
					*((Mutex*) (p_functor_lock))->getMutex()); // lock before queue access

			logger->debug("m_functor_queue.size(): %d", m_functor_queue.size());
			logger->debug("m_workerThreads.size(): %d", m_workerThreads.size());
			logger->debug("max_func_size: %d", max_func_size);

			if (m_functor_queue.size() > max_func_size
					&& m_workerThreads.size() < this->getHighWatermark()) {
				//add new worker thread
				WorkerThread *newWorker = new WorkerThread(&m_functor_queue,
						p_functor_lock);
				logger->debug("create new worker thread[0x%x] ondemand: %i of %i",newWorker,
										m_workerThreads.size() + 1, this->getHighWatermark());
				m_workerThreads.insert(newWorker);
				max_func_size *= 2;
			}

			if (m_functor_queue.size() == 0
					&& m_workerThreads.size() > this->getLowWatermark()) {
				set<MBWorkerThread *>::iterator workerThreads_it =
						m_workerThreads.begin();
				while (workerThreads_it != m_workerThreads.end()) {
					if ((*workerThreads_it)->m_status == worker_idle) {
						WorkerThread *tmp = (WorkerThread*) (*workerThreads_it);
						logger->debug("finish worker thread[0x%x]: (%i of %i)",tmp,
								m_workerThreads.size(),
								this->getHighWatermark());
						tmp->stopThread();
						max_func_size /= 2;
						if (max_func_size == 0)
							max_func_size = 1;
						break;
					}
					++workerThreads_it;
				}
			}

		} else {
			logger->error("p_functor_lock == NULL");
		}

		{
			set<MBWorkerThread *>::iterator workerThreads_it =
					m_workerThreads.begin();
			while (workerThreads_it != m_workerThreads.end()) {
				if ((*workerThreads_it)->m_status == worker_finished) {
					WorkerThread *tmp = (WorkerThread*) (*workerThreads_it);
					logger->debug("delete worker thread[0x%x]: (%i of %i)",tmp,
							m_workerThreads.size(), this->getHighWatermark());

					delete(tmp);
					m_workerThreads.erase(workerThreads_it);
					workerThreads_it = m_workerThreads.begin();
				}
				++workerThreads_it;
			}
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	}
}

void ThreadPool::addFunctor(MBFunctor *work) {
	//logger->debug("add Functor #%i", m_functor_queue.size() + 1);
	Mutex *tmp = (Mutex *)p_functor_lock;
	boost::lock_guard<boost::mutex> lock(*tmp->getMutex());
	m_functor_queue.push_back(work);
}
} /* namespace MB_Gateway */
