//============================================================================
// Name        : MB_Gateway.cpp
// Author      : icke2063
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

#include <ThreadPool.h>
#include <Server.h>
#include <SlaveList.h>
using namespace MB_Gateway;

#include "boost/serialization/singleton.hpp"


int main() {

	boost::serialization::singleton<string>::get_mutable_instance().append("init");

	{
		boost::lock_guard<boost::mutex> lock(boost::serialization::singleton<SlaveList>::get_mutable_instance().p_slavelist_lock->getMutex());

		cout << "SlaveList size:" << boost::serialization::singleton<SlaveList>::get_mutable_instance().slavelist.size() << endl;
	}

	auto_ptr<ThreadPool> pool;
	pool.reset(new ThreadPool());
	pool->setHighWatermark(5);

	//pool->addFunctor(new Server(pool.get(),502));
	pool->addFunctor(new Server(pool.get(),1502));

	while(1){}
	return 0;
}
