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
    Node(int key, int level) : key(key), level(level),left(nullptr), right(nullptr) {
    }
    ~Node() {
     this->left = nullptr;
     this->right = nullptr;
    }

   private:
    Node *left, *right;
  };

  Node* Find(int key);
  Node* Successor(int key);
  Node* Predecessor(int key);
  Node* Insert(int key);
  int Delete(int key);


 private:
  int w;
  std::vector<std::unordered_map<int, Node *>> hash_table;


};
