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

#include <memory>
using namespace std;

#include "boost/serialization/singleton.hpp"

#include <build_options.h>

//#include <ThreadPool.h>
#include <Server.h>
#include <SlaveList.h>
#include <SummerySlave.h>

#include <WebInterface.h>
//#include <MBVirtualRTUSlave.h>
using namespace icke2063::MB_Framework;


//#include <DummyFunctor.h>

#ifdef I2C_SUPPORT
	#include <I2CScanner.h>
#endif

using namespace icke2063::MB_Gateway;


int main() {
int8_t max_functor = 1;
int8_t offset =1;

  printf("crumby\n");

	unique_ptr<Server> default_server(new Server(502));
//	auto_ptr<Server> custom_server;
#ifdef I2C_SUPPORT
	unique_ptr<icke2063::MB_Gateway::I2C::I2C_Scanner> scanner(new icke2063::MB_Gateway::I2C::I2C_Scanner());
#endif
	unique_ptr<WebInterface> webint(new WebInterface());
	boost::serialization::singleton<SlaveList>::get_mutable_instance().addSlave(new SummerySlave(255));

//	

	while (1) {
//		max_functor = 30;
//		cout << "add:" << (int)max_functor  << endl;
//		for(int i=0;i<max_functor;i++){
//			pool->addFunctor(new Dummy_Functor());
//		}
//
//
//		max_functor = max_functor + (offset);
//		offset *= 2;
//
//		if((max_functor + offset) > 100)offset = -1;
//		if(max_functor < 0){offset = 1;max_functor = 1;}


		sleep(1);
	}
	return 0;
}
