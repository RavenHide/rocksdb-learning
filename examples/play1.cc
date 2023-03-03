//
// Created by zhaoyouqing on 2022/11/4.
//

#include <stdio.h>
#include <utility>
#include "limits"
#include "stdint.h"
#include "list"
#include "thread"
#include "pthread.h"

class A {
 public:
  A(){
    printf("create A\n");
  }
  A(int a1, int a2): a(a1), b(a2){
    printf("create A\n");
  }
  ~A() {
    printf("destruction A\n");
  }
  int a;
  int b;
};

A Create() {
//  A a = A();
//  a.a = 10;
//  a.b = 10;
  auto a = A(1, 2);
  a.b = 2;
  return a;
}

void threadARun(int id) {
  printf("id: %d\n", id);
}

int main() {
  A a = Create();
  printf("a: %d, b: %d\n", a.a, a.b);
  a.a = 1;
  a.b = 1;
  printf("a: %d, b: %d\n", a.a, a.b);
  printf("%llu\n", std::numeric_limits<uint64_t>::max());
  printf("%u\n", std::numeric_limits<uint32_t>::max());

  std::list<uint64_t> l{1, 2, 3, 4, 5, 6};
  std::list<uint64_t>::iterator i = l.end();
  --i;
  l.erase(i);
  for (auto e : l) {
    printf("elem: %llu\n", e);
  }

  A a1 = std::move(a);
  printf("a: %d, b: %d\n", a1.a, a1.b);

  std::thread t(threadARun, 123);
  t.join();
}

