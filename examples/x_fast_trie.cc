//
// Created by 赵友清 on 2022/10/23.
//

#include "x_fast_trie.h"
#include "cmath"

// Find: O(1) due to the hash map
XFastTrie::Node * XFastTrie::Find(int key) {
  if (hash_table[w].find(key) == hash_table[w].end()) {
    return nullptr;
  } else {
    return hash_table[w][key];
  }
}

XFastTrie::Node * XFastTrie::Successor(int key) {
  // binary search in hash table levels
  // -------------------
  // | 0 root  |
  // | 1       |
  // | 2       | mid
  // ---------------
  // | 3       | ---
  // | ..      |   |-> = w - mid
  // | w       | ---
  // ----------------
  int low = 0;
  int high = w + 1;
  int mid, prefix = 0;
  Node* tmp = nullptr;
  while (high - low > 1) {
    mid = (high + low) >> 1;
    prefix = key >> (w - mid);
    if (hash_table[mid].find(prefix) == hash_table[mid].end()) {
      high = mid;
    } else {
      low = mid;
      tmp = hash_table[mid][prefix];
    }
  }

  if (tmp == nullptr || tmp->level == 0) {
    return nullptr;
  } else {

  }
}

int main() {
  int input = 4;
  auto a = std::log2(input);
  auto b = log2Result(input);
  printf("input: %d, a: %lf, b: %d\n", input, a, b);

  input = 8;
  a = std::log2(input);
  b = log2Result(input);
  printf("input: %d, a: %lf, b: %d\n", input, a, b);

  input = 9;
  a = std::log2(input);
  b = log2Result(input);
  printf("input: %d, a: %lf, b: %d\n", input, a, b);
  std::unordered_map<int, int> m_map{{1, 10}, {2, 20}};

  printf("1: %d\n", m_map[1]);
  printf("1: %d\n", m_map.at(1));
  printf("3: %d\n", m_map[3]);
}