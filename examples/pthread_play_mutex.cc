//
// Created by 赵友清 on 2022/10/2.
//

#include "pthread_play_mutex.h"

void PthreadPlayMutex::Lock() {
  pthread_mutex_lock(&mu_);
}

void PthreadPlayMutex::Unlock() {
  pthread_mutex_unlock(&mu_);
}

void PthreadPlayMutex::Wait() {
  pthread_cond_wait(&cv_, &mu_);
}

void PthreadPlayMutex::Signal() {
  pthread_cond_signal(&cv_);
}

void PthreadPlayMutex::SignalAll() {
  pthread_cond_broadcast(&cv_);
}