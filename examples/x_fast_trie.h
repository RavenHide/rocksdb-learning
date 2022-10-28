//
// Created by 赵友清 on 2022/10/23.
//

#pragma once
#include <unordered_map>
#include <vector>

int log2Result(int input) {
 int count = 0;
  input = input >> 1;
  while (input > 0) {
   ++count;
   input = input >> 1;
 }
 return count;
}

class XFastTrie {
 public:
  explicit XFastTrie(int u) {
    this->w = log2Result(u);
    hash_table.reserve(this->w + 1);
    hash_table.assign(this->w + 1, std::unordered_map<int, Node*>());
    Node root(0, 0);
    hash_table[0][0] = &root;
  }

  class Node {
   public:
    const int key;
    const int level;
    Node *left, *right;

    Node(int key, int level) : key(key), level(level),left(nullptr), right(nullptr) {}
    ~Node() {
     this->left = nullptr;
     this->right = nullptr;
    }
  };

  Node* binarySearch(int key) {
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
    return tmp;
  }

  Node* leftMostLeafNode(Node* node) {
    while (node->level < w) {
      if (node->left != nullptr) {
        node = node->left;
      } else {
        node = node->right;
      }
    }
    return node;
  }


  Node* rightMostLeafNode(Node* node) {
    while (node->level < w) {
      if (node->right != nullptr) {
        node = node->right;
      } else {
        node = node->left;
      }
    }
    return node;
  }

  bool Find(int key, Node** target);
  Node* Successor(int key);
  Node* Predecessor(int key);
  bool Insert(int key);
  int Delete(int key);


 private:
  int w;
  std::vector<std::unordered_map<int, Node *>> hash_table;


};
