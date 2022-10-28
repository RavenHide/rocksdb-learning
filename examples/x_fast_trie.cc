//
// Created by 赵友清 on 2022/10/23.
//

#include "x_fast_trie.h"

#include "cmath"
#include "limits"

// Find: O(1) due to the hash map
bool XFastTrie::Find(int key, XFastTrie::Node** target) {
  if (hash_table[w].find(key) == hash_table[w].end()) {
    return false;
  } else {
    *target = hash_table[w][key];
    return true;
  }
}

// Successor return the smallest node greater than or equal to key, null is
// target node not exist in trie
XFastTrie::Node* XFastTrie::Successor(int key) {
  XFastTrie::Node* tmp = this->binarySearch(key);

  if (tmp == nullptr || tmp->level == 0) {
    return nullptr;
  } else {
    // hit leaf node, key is in the trie
    if (tmp->level == w) {
      return tmp;

    } else {
      // if key is not in the trie, get it's descendant ptr
      if ((key >> (w - tmp->level - 1)) & 1) {
        tmp = tmp->right;
      } else {
        tmp = tmp->left;
      }

      if (tmp->key < key) {
        return tmp->right;
      } else {
        return tmp;
      }
    }
  }
}

// Predecessor return the greatest node letter than or equal to the key
XFastTrie::Node* XFastTrie::Predecessor(int key) {
  auto tmp = this->binarySearch(key);
  if (tmp == nullptr || tmp->level == 0) {
    return nullptr;

  } else {
    if (tmp->level == w) {
      return tmp;
    } else {
      // get it's descendant ptr
      if ((key >> (w - tmp->level - 1)) & 1) {
        tmp = tmp->right;
      } else {
        tmp = tmp->left;
      }

      if (tmp->key > key) {
        return tmp->left;
      } else {
        return tmp;
      }
    }
  }
}

bool XFastTrie::Insert(int key) {
  if (hash_table[w].find(key) != hash_table[w].end()) {
    // node has exist in trie.
    return false;

  } else {
    // link node with it's successor and predecessor
    auto inserted_node = new Node(key, w);
    auto successor = Successor(key);
    if (successor != nullptr) {
      if (successor->level != w) {
        printf("invalid level, key: %d, level: %d", successor->key,
               successor->level);
      }
      inserted_node->left = successor->left;
      successor->left = inserted_node;
      inserted_node->right = successor;
    }

    auto predecessor = Predecessor(key);
    if (predecessor != nullptr) {
      if (predecessor->level != w) {
        printf("invalid level, key: %d, level: %d", predecessor->key,
               predecessor->level);
      }
      inserted_node->right = predecessor->right;
      predecessor->right = inserted_node;
      inserted_node->left = predecessor;
    }

    // update hash table and trie
    // -------------------
    // | 0 root  |
    // | 1       | start
    // | 2       |
    // | 3       |
    // | 4       |
    // | ..      |
    // | w-1     | end
    // | w       |
    // ----------------
    int prefix = 0;
    for (int level = 1; level < w; ++level) {
      prefix = key >> (w - level);
      // if prefix not exists in trie, create a new node into trie.
      if (hash_table[level].find(prefix) == hash_table[level].end()) {
        auto new_node = new Node(0, level);
        // add to the hash table
        hash_table[level][prefix] = new_node;
        // update trie;
        if (prefix & 1) {
          hash_table[level - 1][prefix >> 1]->right = new_node;
        } else {
          hash_table[level - 1][prefix >> 1]->left = new_node;
        }
      }
    }
    hash_table[w][key] = inserted_node;
    if (key & 1) {
      hash_table[w - 1][key >> 1]->right = inserted_node;
    } else {
      hash_table[w - 1][key >> 1]->left = inserted_node;
    }

    // update descendant ptr
    for (int level = w - 1; level > 0; --level) {
      prefix = key >> (w - level);
      auto hit_node = hash_table[level][prefix];
      if (hit_node->left == nullptr) {
        hit_node->left = leftMostLeafNode(hit_node->right);
      }
      if (hit_node->right == nullptr) {
        hit_node->right = rightMostLeafNode(hit_node->left);
      }
    }
    return true;
  }
}

int main() {
  auto trie = XFastTrie(std::numeric_limits<int>().max());
  int key = 100;
  trie.Insert(key);
  XFastTrie::Node* hitNode = nullptr;
  bool ok = trie.Find(key, &hitNode);
  if (!ok) {
    printf("key %d is not in the x-fast-trie\n", key);
    return 1;
  }
  printf("hit node: %d, level: %d\n", hitNode->key, hitNode->level);

  ok = trie.Find(101, nullptr);
  if (ok) {
    printf("found a ghost node %d in x-fast-trie\n", 101);
    return 1;
  }
  printf("101 not found in x-fast-trie\n");
  return 0;
}