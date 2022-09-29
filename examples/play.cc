//
// Created by zhaoyouqing on 2022/8/30.
//

#include <inttypes.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <atomic>

#include "stdexcept"
#include "unordered_map"
#include "array"

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

    fprintf(stdout, "address: %p, i: %llu, gen_str: %s \n", &str, i,
            str.c_str());
  }
}

void print_vector_str(std::vector<std::string>& str_list) {
  for (auto s : str_list) {
    printf("%s ", s.c_str());
  }
  printf("\n");
}

void remove_duplicate() {
  std::vector<std::string> str_list{"2", "2", "1", "1", "4", "4",
                                    "1", "3", "2", "3", "5"};
  std::sort(str_list.begin(), str_list.end());
  print_vector_str(str_list);
  // duplicate
  auto ip = std::unique(str_list.begin(), str_list.end());
  print_vector_str(str_list);
  str_list.erase(ip, str_list.end());
  print_vector_str(str_list);

  //  for (auto i = li; i != str_list.end(); ++i) {
  //   printf("%s ", i->c_str());
  //  }
  //  printf("\n");
}

void play_unordered_map() {
  std::unordered_map<std::string, uint64_t> m{{"1", 1}, {"2", 2}, {"3", 3}};
  for (auto i : m) {
    printf("%s: %llu, ", i.first.c_str(), i.second);
  }
  printf("\n");
  auto hit = m.find("5");
  if (hit != m.end()) {
    printf("%s: %llu\n", hit->first.c_str(), hit->second);
  } else {
    printf("not found key\n");
  }
  try {
    auto hit_val = m.at("5");
    printf("hit val: %llu\n", hit_val);
  } catch (std::out_of_range) {
    printf("not found \n");
  }
}

#define XXPH_NAMESPACE ROCKSDB_
#define XXPH_CAT(A,B) A##B
#define XXPH_NAME2(A,B) XXPH_CAT(A,B)
#define XXPH3_64bits_withSeed XXPH_NAME2(XXPH_NAMESPACE, XXPH3_64bits_withSeed)

static uint64_t XXPH3_64bits_withSeed(uint64_t a, uint64_t b) {
  return a + b;
}


#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x)   (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

void atomic_play() {
  // atomic play
  std::atomic<std::string*> atomic_str;
  std::string str = "123";
  atomic_str.store(&str);
  auto p_str = atomic_str.load(std::memory_order_relaxed);
  printf("first get from atomic_str, p_str: %s\n", p_str->c_str());

  std::string str1 = "456";
  atomic_str.store(&str1);

  std::string set_str = "789";
  bool ok = atomic_str.compare_exchange_weak(p_str, &set_str);
  printf("exchange ok: %d, compare_exchange_weak: p_str: %s\n", ok, p_str->c_str());

  ok = atomic_str.compare_exchange_weak(p_str, &set_str);
  if (ok) {
    printf("is p_str a nullptr? %d\n", p_str == nullptr);
  }

}

// todo play pthread_cond_wait and pthread_cond_signal
void pthread_play() {

}

int main() {
  //  test_key();
  //  remove_duplicate();
//  play_unordered_map();

  size_t ts_sz = 8;
  std::string dummy_ts(ts_sz, '\0');
  printf("%llu\n", XXPH3_64bits_withSeed(1, 2));
  std::array<std::string, 2> str_list{"1", "2"};
  auto data = str_list.data();
  for (int i = 0; i < str_list.size(); ++i) {
    printf("val: %s\n", data->c_str());
    ++data;
  }
  data = nullptr;
  if UNLIKELY (true) {
    printf("11111111\n");
  }

  if LIKELY (true) {
    printf("2222\n");
  }

  atomic_play();
}
