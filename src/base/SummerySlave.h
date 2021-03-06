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

#include <build_options.h>

/** C++11 */
  #include <memory>
  #include <thread>
  #include <condition_variable>
  #include <mutex>

//own lib
#include <VirtualRTUSlave.h>

#include <ThreadPool.h>


#define SLAVE_TYPE_SUMMERY	0x01

#define DEFAULT_SUMMERY_TIMEOUT_MS	1000
#define DEFAULT_SUMMERY_ADDR	255
#define DEFAULT_SUMMERY_COUNT	256
#define DEFAULT_SUMMERY_VALUE	0

namespace icke2063 {
namespace MB_Gateway {

class SummerySlave:
	public VirtualRTUSlave,
	public std::enable_shared_from_this<SummerySlave> {
  
  class SummerySlaveFunctor:
	  public icke2063::threadpool::Functor,
	  public std::enable_shared_from_this<SummerySlaveFunctor>{
  public:
    SummerySlaveFunctor(std::shared_ptr<SummerySlave> slave):m_slave(slave){}
    virtual ~SummerySlaveFunctor(){}

    virtual void functor_function(void);
  private:
    std::shared_ptr<SummerySlave> m_slave;
  };
  
  friend class SummerySlaveFunctor;
  
public:
	SummerySlave(std::shared_ptr<icke2063::threadpool::ThreadPool> delayed_pool, uint8_t SlaveAddr = DEFAULT_SUMMERY_ADDR, unsigned int timeout = DEFAULT_SUMMERY_TIMEOUT_MS);
	virtual ~SummerySlave();
	virtual uint8_t getType( void ){return SLAVE_TYPE_SUMMERY;}
	void startFunctor(void);

private:
	bool init(void);

	//unique_ptr<thread> p_scanner_thread;
	bool m_running;
	
	std::condition_variable m_Condition;  // Condition variable for timed_wait
	std::mutex m_Mutex;                   // Mutex
	unsigned int m_timeout;
	std::shared_ptr<icke2063::threadpool::ThreadPool> m_delayed_pool;
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* SUMMERYSLAVE_H_ */
