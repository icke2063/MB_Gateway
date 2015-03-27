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

#include <build_options.h>

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

using namespace icke2063::MB_Framework;

#include <ThreadPool.h>

#include <crumby_logging_macros.h>
#include <modbus_logging_macros.h>
#include <threadpool_logging_macros.h>
#include <i2c_logging_macros.h>


#ifdef I2C_SUPPORT
	#include <I2CScanner.h>
#endif

using namespace icke2063::MB_Gateway;

void crumby_logfn(uint8_t loglevel, const char *file, const int line, const char *fn, const char *format, va_list args)
{
	printf(format, args);
	printf("\n");
}

int main()
{
	crumby_SET_LOG_FN(crumby_logfn);
	modbus_SET_LOG_FN(crumby_logfn);
	threadpool_SET_LOG_FN(crumby_logfn);
	i2c_SET_LOG_FN(crumby_logfn);


//	crumby_SET_LOG_LEVEL(LOG_INFO);
//	modbus_SET_LOG_LEVEL(LOG_INFO);
//	threadpool_SET_LOG_LEVEL(LOG_INFO);
//	i2c_SET_LOG_LEVEL(LOG_INFO);


	shared_ptr<icke2063::threadpool::ThreadPool> pool(new icke2063::threadpool::ThreadPool(5));
	pool->setHighWatermark(10);
	pool->setLowWatermark(3);
	
	std::auto_ptr<Server> default_server(new Server(502,pool));
	  

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
