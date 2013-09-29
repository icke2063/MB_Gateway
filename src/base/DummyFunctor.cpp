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

Dummy_Functor::Dummy_Functor() {
	// TODO Auto-generated constructor stub

}

Dummy_Functor::~Dummy_Functor() {
	// TODO Auto-generated destructor stub
}

void Dummy_Functor::functor_function(void){
	usleep(10000);
}

} /* namespace MB_Gateway */
} /* namespace icke2063 */
