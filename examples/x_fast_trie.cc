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
  if (tmp != nullptr) {
    printf("binary_search: key: %d, level: %d, result: %d\n", key, tmp->level, tmp->key);
    printf("binary_search: key: %d, level: %d, left_child_level: %d, left_child: %d, "
        "right_child_level: %d, right_child: %d\n",
        key, tmp->level, tmp->left->level, tmp->left->key,
        tmp->right->level, tmp->right->key);
  }
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

int XFastTrie::Delete(int key) {
  // check key is weather in trie
  XFastTrie::Node* node = nullptr;
  bool ok = this->Find(key, &node);
  if (!ok) {
    return 0;
  } else {
    // update double-link list
    if (node->left != node && node->right != node) {
      if (node->left != nullptr) {
        node->left->right = node->right;
      }
      if (node != node->right && node->right != nullptr) {
        node->right->left = node->left;
      }
    } else {
      if (node->left == node && node->right != node) {
        node->right->left = nullptr;
      } else if (node->right == node && node->left != node) {
        node->left->right = nullptr;
      }
    }
    node = nullptr;
    // traverse this trie from bottom, and delete all internal node
    int prefix = 0;
    for (int level = w; level > 0; --level) {
      prefix = key >> (w - level);
      // clear node
      hash_table[level].erase(prefix);
      auto parent_key = prefix >> 1;
      auto parent = hash_table[level - 1][parent_key];
      XFastTrie::Node* brother_node = nullptr;
      XFastTrie::Node* hit_node = nullptr;
      if (prefix & 1) {
        // right child
        hit_node = parent->right;
        parent->right = nullptr;
        brother_node = parent->left;
      } else {
        hit_node = parent->left;
        parent->left = nullptr;
        brother_node = parent->right;
      }
      printf("level: %d, brother: %d, key: %d\n", level, brother_node->key, prefix);
      // recycle mem
      delete hit_node;

      // stop traverse if parent is not a orphan after node deleted.
      if (brother_node != hit_node &&
          brother_node->level == level &&
          (brother_node->key >> 1) == parent_key) {
        break;
      }
    }

    // update descendant node
    for (int level = w - 1; level > 0; --level) {
      prefix = key >> (w - level);
      if (hash_table[level].find(prefix) != hash_table[level].end()) {
        auto hit_node = hash_table[level][prefix];
        auto has_right_child = XFastTrie::Node::HasRightChild(hit_node);
        auto has_left_child = XFastTrie::Node::HasLeftChild(hit_node);
        if (has_left_child && !has_right_child) {
          hit_node->right = rightMostLeafNode(hit_node->left);
        } else if(!has_left_child && has_right_child) {
          hit_node->left = leftMostLeafNode(hit_node->right);
        } else {
          // if both children are all exist, nothing to do here
        }

      }
    }

  }

  return 1;
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
        auto new_node = new Node(prefix, level);
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
  printf("insert node: %d into trie\n", key);
  XFastTrie::Node* hit_node = nullptr;
  printf("find node: %d in this trie\n", key);
  bool ok = trie.Find(key, &hit_node);
  if (!ok) {
    printf("key %d is not in the x-fast-trie\n", key);
    return 1;
  }
  printf("hit node: %d, level: %d\n", hit_node->key, hit_node->level);

  printf("find node: 101 in this trie\n");
  ok = trie.Find(101, nullptr);
  if (ok) {
    printf("found a ghost node %d in x-fast-trie\n", 101);
    return 1;
  }
  printf("key: 101 not found in x-fast-trie\n");

  printf("insert node: %d into trie\n", 103);
  trie.Insert(103);
  hit_node = nullptr;
  printf("search successor of key 99 in this trie\n");
  hit_node = trie.Successor(99);
  if (hit_node == nullptr) {
    printf("invalid successor result\n");
    return 1;
  }
  if (hit_node->key != 100) {
    printf("incorrect successor node\n");
    return 1;
  }
  printf("successor of key 99 is %d\n", hit_node->key);

  hit_node = trie.Predecessor(99);
  if (hit_node != nullptr) {
    printf("invalid predecessor result\n");
    return 1;
  }

  printf("insert node: %d into trie\n", 90);
  trie.Insert(90);
  printf("search predecessor of key 99 in this trie\n");
  hit_node = trie.Predecessor(99);
  if (hit_node == nullptr) {
    printf("invalid predecessor result\n");
    return 1;
  }
  printf("predecessor of key of 99 is %d\n", hit_node->key);


  // 90  100 103
  ok = trie.Delete(98);
  if (ok) {
    printf("invalid delete\n");
    return 1;
  }

  hit_node = trie.Successor(99);
  printf("successor of key 99 is %d\n", hit_node->key);

  printf("delete of key 100 \n");
  ok = trie.Delete(100);
  if (!ok) {
    printf("invalid delete\n");
    return 1;
  }
  hit_node = trie.Successor(98);
  if (hit_node == nullptr) {
    printf("invalid delete for successor\n");
    return 1;
  }
  printf("successor of key 99 is %d\n", hit_node->key);


  hit_node = trie.Predecessor(101);
  if (hit_node == nullptr) {
    printf("invalid delete for predecessor\n");
    return 1;
  }
  printf("predecessor of key 101 is %d\n", hit_node->key);
  return 0;
}