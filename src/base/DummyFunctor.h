/**
 * @file   DummyFunctor.h
 * @Author icke
 * @date   24.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef DUMMYFUNCTOR_H_
#define DUMMYFUNCTOR_H_

#include <ThreadPool.h>
#include <sys/time.h>

using namespace icke2063::common_cpp;

namespace icke2063 {
namespace MB_Gateway {

class Dummy_Functor: public Functor {
public:
	Dummy_Functor(){
	  gettimeofday(&creation_time,NULL);
	  printf("Dummy_Functor\n");
	  printTimestamp(&creation_time);
	};
	virtual ~Dummy_Functor(){
	  printf("~Dummy_Functor\n");
	};
	virtual void functor_function(void);
private:
  struct timeval creation_time;
  
  
  void printTimestamp (struct timeval *timestamp);
  
};


} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* DUMMYFUNCTOR_H_ */
