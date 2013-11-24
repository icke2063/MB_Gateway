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

void Dummy_Functor::printTimestamp (struct timeval *timestamp){
  time_t nowtime;
  struct tm *nowtm;
  char tmbuf[64], buf[64];

  nowtime = timestamp->tv_sec;
  nowtm = localtime(&nowtime);
  strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
  snprintf(buf, sizeof buf, "%s.%06d", tmbuf, timestamp->tv_usec);
  printf("%s \n",buf);
  }

void Dummy_Functor::functor_function(void){
  struct timeval current;
  
  gettimeofday(&current,NULL);
  printf("functor_function\n");
  printf("priority:%d\n",getPriority());
  printf("creation time:");printTimestamp(&creation_time);
  printf("current time:");printTimestamp(&current);
  
  sleep(1);
  printf("finished\n");
}

} /* namespace MB_Gateway */
} /* namespace icke2063 */
