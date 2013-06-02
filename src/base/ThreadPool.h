/**
 * @file   ThreadPool.h
 * @Author icke2063
 * @date   28.05.2013
 * @brief  Brief description of file.
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

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <MBThreadPool.h>
using namespace MB_Framework;

#include <boost/thread.hpp>

#include <Mutex.h>
#include <Logger.h>

namespace MB_Gateway {

class ThreadPool: public MB_Framework::MBThreadPool ,public Logger{
public:
	ThreadPool();
	virtual ~ThreadPool();
	virtual void scheduler(void);
	virtual void addFunctor(MBFunctor *work);

	void scheduler2(void);

private:
	bool running;

	/**
	 * Scheduler is used for creating and scheduling the WorkerThreads.
	 * - on high usage (many unhandled functors in queue) create new threads until HighWatermark limit
	 * - on low usage and many created threads -> delete some to save resources
	 */
	boost::thread *p_scheduler_thread;
	Mutex* p_functor_lock;				//lock for functor list
};

} /* namespace MB_Gateway */
#endif /* THREADPOOL_H_ */
