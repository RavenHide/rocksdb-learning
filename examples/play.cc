//
// Created by zhaoyouqing on 2022/8/30.
//

#include <cstdio>
#include <cstdint>
#include <string>
#include <inttypes.h>


inline bool IsLittleEndian() {
  uint32_t x = 1;
  return *reinterpret_cast<char*>(&x) != 0;
}

static std::string Key(uint64_t k) {
//  printf("k address: %p, %lu\n", &k, sizeof(k));
  std::string ret;
  if (IsLittleEndian()) {
    ret.append(reinterpret_cast<char*>(&k), sizeof(k));
//    printf("ret: %s, 111111111\n", ret.c_str());
  } else {
    char buf[sizeof(k)];
    for (size_t i = 0; i < sizeof(k); ++i) {
      buf[i] = (k >> (i * 8)) & 0xff;
    }
    ret.append(buf, sizeof(k));
  }

//  size_t i = 0, j = ret.size() - 1;
//  while (i < j) {
//    char tmp = ret[i];
//    ret[i] = ret[j];
//    ret[j] = tmp;
//    ++i;
//    --j;
//  }
  printf("ret: %s\n", ret.c_str());
  return ret;
}

void test_key() {
  for (uint64_t i = 0; i < 300; ++i) {
    std::string str = Key(i);

    fprintf(stdout, "address: %p, i: %llu, gen_str: %s \n", &str, i, str.c_str());
  }
}

int main() {
  test_key();
}
