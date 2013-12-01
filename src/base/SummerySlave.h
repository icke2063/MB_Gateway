/**
 * @file   SummerySlave.h
 * @Author icke
 * @date   03.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef SUMMERYSLAVE_H_
#define SUMMERYSLAVE_H_

//std lib
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
  #include <memory>
  #include <thread>
  #include <condition_variable>
  #include <mutex>
  using namespace std;
#endif


//own lib
#include <VirtualRTUSlave.h>
#include <Logger.h>

#include <ThreadPool.h>


#define SLAVE_TYPE_SUMMERY	0x01

#define DEFAULT_SUMMERY_TIMEOUT_MS	1000
#define DEFAULT_SUMMERY_ADDR	255
#define DEFAULT_SUMMERY_COUNT	256
#define DEFAULT_SUMMERY_VALUE	0

using namespace icke2063::MB_Framework;
using namespace icke2063::common_cpp;

namespace icke2063 {
namespace MB_Gateway {

class SummerySlave: public VirtualRTUSlave, public Logger, public enable_shared_from_this<SummerySlave> {
  
  class SummerySlaveFunctor: public Functor,public enable_shared_from_this<SummerySlaveFunctor>{
  public:
    SummerySlaveFunctor(shared_ptr<SummerySlave> slave):m_slave(slave){}
    virtual ~SummerySlaveFunctor(){}

    virtual void functor_function(void);
  private:
    shared_ptr<SummerySlave> m_slave;
  };
  
  friend class SummerySlaveFunctor;
  
public:
	SummerySlave(shared_ptr<ThreadPool> delayed_pool, uint8_t SlaveAddr = DEFAULT_SUMMERY_ADDR, unsigned int timeout = DEFAULT_SUMMERY_TIMEOUT_MS);
	virtual ~SummerySlave();
	virtual uint8_t getType( void ){return SLAVE_TYPE_SUMMERY;}
	void startFunctor(void);

private:
	bool init(void);

	//unique_ptr<thread> p_scanner_thread;
	bool m_running;
	
	condition_variable m_Condition;  // Condition variable for timed_wait
	mutex m_Mutex;                   // Mutex
	unsigned int m_timeout;
	shared_ptr<ThreadPool> m_delayed_pool;
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* SUMMERYSLAVE_H_ */
