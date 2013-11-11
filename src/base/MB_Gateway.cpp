//============================================================================
// Name        : MB_Gateway.cpp
// Author      : icke2063
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <auto_ptr.h>
using namespace std;

#include "boost/serialization/singleton.hpp"

#include <build_options.h>

#include <ThreadPool.h>
#include <Server.h>
#include <WebInterface.h>
#include <SlaveList.h>
#include <MBVirtualRTUSlave.h>
using namespace icke2063::MB_Framework;

#include <SummerySlave.h>
#include <DummyFunctor.h>

#ifdef I2C_SUPPORT
	#include <I2CScanner.h>
#endif

using namespace icke2063::MB_Gateway;

int main() {
int8_t max_functor = 1;
int8_t offset =1;

	auto_ptr<Server> default_server;
	auto_ptr<Server> custom_server;
	auto_ptr<icke2063::MB_Gateway::I2C::I2C_Scanner> scanner;


	default_server.reset(new Server(502));

	boost::serialization::singleton<SlaveList>::get_mutable_instance().addSlave(new SummerySlave(255));

	scanner.reset(new icke2063::MB_Gateway::I2C::I2C_Scanner());

	WebInterface webint;

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
