//============================================================================
// Name        : MB_Gateway.cpp
// Author      : icke2063
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef ICKE2063_CRUMBY_NO_CPP11
  #include <memory>
  using namespace std;
#else
  #include <boost/scoped_ptr.hpp>
  using namespace boost;
#endif


#include <sys/time.h>

#include "boost/serialization/singleton.hpp"

#include <build_options.h>

#include <Server.h>
#include <SlaveList.h>
#include <SummerySlave.h>

#include <WebInterface.h>
//#include <MBVirtualRTUSlave.h>
using namespace icke2063::MB_Framework;

#include <ThreadPool.h>


#ifdef I2C_SUPPORT
	#include <I2CScanner.h>
#endif

using namespace icke2063::MB_Gateway;

int main() {

	shared_ptr<ThreadPool> pool(new ThreadPool());
	pool->setHighWatermark(10);
	pool->setLowWatermark(1);
	

#ifndef ICKE2063_CRUMBY_NO_CPP11
	unique_ptr<Server> default_server(new Server(502,pool));
#else
	scoped_ptr<Server> default_server(new Server(502,pool));
#endif
	  

	//	unique_ptr<Server> custom_server;
#ifdef I2C_SUPPORT
#ifndef ICKE2063_CRUMBY_NO_CPP11
	unique_ptr<icke2063::MB_Gateway::I2C::I2C_Scanner> scanner(new icke2063::MB_Gateway::I2C::I2C_Scanner());
#else
	scoped_ptr<icke2063::MB_Gateway::I2C::I2C_Scanner> scanner(new icke2063::MB_Gateway::I2C::I2C_Scanner());
#endif
#endif
	shared_ptr<SummerySlave> sum = shared_ptr<SummerySlave>(new SummerySlave(pool,255));
	sum->startFunctor();
	boost::serialization::singleton<SlaveList>::get_mutable_instance().addSlave(sum);

#ifndef ICKE2063_CRUMBY_NO_CPP11
	unique_ptr<WebInterface> webint(new WebInterface());
#else
	scoped_ptr<WebInterface> webint(new WebInterface());
#endif

	

	while (1) {
		sleep(5);
	}
	return 0;
}
