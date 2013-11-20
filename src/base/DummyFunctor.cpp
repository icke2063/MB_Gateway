/**
 * @file   DummyFunctor.cpp
 * @Author icke
 * @date   24.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include "DummyFunctor.h"
#include "unistd.h"

namespace icke2063 {
namespace MB_Gateway {


void Dummy_Functor::functor_function(void){
	printf("Dummy_Functor\n");
	usleep(10000);
	printf("finished\n");
}

} /* namespace MB_Gateway */
} /* namespace icke2063 */
