//
// Created by 赵友清 on 2022/10/2.
//

#pragma once

//#ifndef ROCKSDB_PTHREAD_PLAY_MUTEX_H
//#define ROCKSDB_PTHREAD_PLAY_MUTEX_H

#include "pthread.h"

class PthreadPlayMutex {
 public:
  PthreadPlayMutex() {
    // init mutex
    pthread_mutex_init(&mu_, nullptr);
    pthread_cond_init(&cv_, nullptr);
  }
  ~PthreadPlayMutex() {
    pthread_cond_destroy(&cv_);
    pthread_mutex_destroy(&mu_);
  }
  void Lock();
  void Unlock();
  void Wait();

  void Signal();
  void SignalAll();
 private:
  pthread_mutex_t mu_;
  pthread_cond_t cv_;
};

//#endif  // ROCKSDB_PTHREAD_PLAY_MUTEX_H
