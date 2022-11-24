//
// Created by zhaoyouqing on 2022/11/4.
//

#include <stdio.h>
#include <utility>
#include "limits"
#include "stdint.h"

class A {
 public:
  A(){
    printf("create A\n");
  }
  A(int a1, int a2): a(a1), b(a2){
    printf("create A\n");
  }
  ~A() {
    a = 0;
    b = 0;
    printf("destruction A\n");
  }
  int a;
  int b;
};

A&& Create() {
//  A a = A();
//  a.a = 10;
//  a.b = 10;
  return std::move(A(1, 2));
}

int main() {
  A&& a = Create();
  printf("a: %d, b: %d\n", a.a, a.b);
  a.a = 1;
  a.b = 1;
  printf("a: %d, b: %d\n", a.a, a.b);
  printf("%lu\n", std::numeric_limits<uint64_t>::max());
  printf("%u\n", std::numeric_limits<uint32_t>::max());
}

