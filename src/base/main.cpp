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

 //boost
#include "boost/serialization/singleton.hpp"
#include "boost/program_options.hpp"
namespace po = boost::program_options;

#include <build_options.h>

#include <Server.h>
#include <SlaveList.h>
#include <SummerySlave.h>

using namespace icke2063::MB_Framework;

#include <ThreadPool.h>

#include <crumby_logging_macros.h>
#include <modbus_logging_macros.h>
#include <threadpool_logging_macros.h>



#ifdef CRUMBY_I2C_SUPPORT
	#include <I2CScanner.h>
	#include <i2c_logging_macros.h>
#endif

#ifdef CRUMBY_WEBINTERFACE
	#include <WebInterface.h>
	#include <pages_logging_macros.h>
#endif

using namespace icke2063::MB_Gateway;

FILE *logfile = NULL;

void crumby_logfn(uint8_t loglevel, const char *file, const int line, const char *fn, const char *format, va_list args)
{
	if(logfile != NULL)
	{
	    fprintf(logfile, "crumby[%d]: ", loglevel);
	    vfprintf(logfile, format, args);
	    fprintf(logfile, "\n");
	    fflush( logfile );
	}
}

int main(int argc, const char *argv[])
{

	logfile = stdout;


	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help,h", "produce help message")
	    ("mb_port", po::value<int>(), "extra custom modbus server port")
	    ("crumby_loglevel", po::value<int>(), "")
	    ("modbus_loglevel", po::value<int>(), "")
	    ("tp_loglevel", po::value<int>(), "")
	    ;

#ifdef CRUMBY_I2C_SUPPORT
	desc.add_options()
	    ("i2c_master", po::value<std::string>(), "path to I2C master")
	    ("i2c_loglevel", po::value<int>(), "I2C loglevel[0..7]")
	    ;
#endif
	    
#ifdef CRUMBY_WEBINTERFACE
	    
	desc.add_options()
	    ("pages_loglevel", po::value<int>(), "pages loglevel[0..7]")
	    ;
#endif
	    
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
	   std:: cout << desc << "\n";
	    return 1;
	}

	crumby_SET_LOG_FN(crumby_logfn);
	modbus_SET_LOG_FN(crumby_logfn);
	threadpool_SET_LOG_FN(crumby_logfn);

	if(vm.count("crumby_loglevel")){
		crumby_SET_LOG_LEVEL(vm["crumby_loglevel"].as<int>());
	}

	if(vm.count("modbus_loglevel")){
		modbus_SET_LOG_LEVEL(vm["modbus_loglevel"].as<int>());
	}

	if(vm.count("tp_loglevel")){
		threadpool_SET_LOG_LEVEL(vm["tp_loglevel"].as<int>());
	}

	shared_ptr<icke2063::threadpool::ThreadPool> pool(new icke2063::threadpool::ThreadPool(5));
	pool->setHighWatermark(10);
	pool->setLowWatermark(3);
	
	std::auto_ptr<Server> default_server(new Server(502,pool));

	if(vm.count("mb_port"))
	{
		std::auto_ptr<Server> custom_server(new Server(vm["mb_port"].as<int>() ,pool));
	}
	//

#ifdef CRUMBY_I2C_SUPPORT
	std::string i2c_master_path("/dev/null");

	if(vm.count("i2c_master")){
		i2c_master_path = vm["i2c_master"].as<std::string>();
	}

	if(vm.count("i2c_loglevel")){
		i2c_SET_LOG_LEVEL(vm["i2c_loglevel"].as<int>());
	}

	i2c_SET_LOG_FN(crumby_logfn);
	std::auto_ptr<icke2063::MB_Gateway::I2C::I2C_Scanner> scanner(new icke2063::MB_Gateway::I2C::I2C_Scanner(i2c_master_path, 10000));
#endif

	shared_ptr<SummerySlave> sum = shared_ptr<SummerySlave>(new SummerySlave(pool,255));
	sum->startFunctor();
	boost::serialization::singleton<SlaveList>::get_mutable_instance().addSlave(sum);

#ifdef CRUMBY_WEBINTERFACE
	pages_SET_LOG_FN(crumby_logfn);
	
	if(vm.count("pages_loglevel")){
		pages_SET_LOG_LEVEL(vm["pages_loglevel"].as<int>());
	}
	
	std::auto_ptr<WebInterface> webint(new WebInterface());
#endif
	
	while (1)
	{
		sleep(5);
	}
	return 0;
}
