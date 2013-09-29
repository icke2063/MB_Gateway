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

using namespace icke2063::common_cpp;

namespace icke2063 {
namespace MB_Gateway {

class Dummy_Functor: public FunctorInt {
public:
	Dummy_Functor();
	virtual ~Dummy_Functor();
	virtual void functor_function(void);
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* DUMMYFUNCTOR_H_ */
