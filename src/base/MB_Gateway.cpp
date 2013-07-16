//============================================================================
// Name        : MB_Gateway.cpp
// Author      : icke2063
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

#include "boost/serialization/singleton.hpp"

#include <build_options.h>

#include <ThreadPool.h>
#include <Server.h>
#include <SlaveList.h>
#include <MBVirtualRTUSlave.h>
using namespace MB_Framework;
#include <SummerySlave.h>
#ifdef I2C_SUPPORT
#include <I2CScanner.h>
#endif
using namespace MB_Gateway;

int main() {

	auto_ptr<ThreadPool> pool;
	auto_ptr<MB_Gateway::I2C::I2C_Scanner> scanner;
	pool.reset(new ThreadPool());
	pool->setHighWatermark(5);

	cout << "ThreadPool" << std::hex << pool.get() << endl;

	pool->addFunctor(new Server(pool.get(), 502));
	pool->addFunctor(new Server(pool.get(), 1502));

	boost::serialization::singleton<SlaveList>::get_mutable_instance().addSlave(
			255, new SummerySlave(255));
	scanner.reset(new MB_Gateway::I2C::I2C_Scanner());

	while (1) {
	}
	return 0;
}
