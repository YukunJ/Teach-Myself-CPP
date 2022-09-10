/**
 * skiplist.h
 * This is the implementation of a simple SkipList data structure using linked
 * list It supports three main APIs: SkipSearch, SkipInsert and SkipRemove We
 * use linked node of 4-directional links to connect the list and maintain the
 * top-left header
 */
#ifndef KVSTORE_SKIPLIST_H
#define KVSTORE_SKIPLIST_H

#include <math.h>
#include <random>
#include <vector>
#include <mutex>

namespace kvstore {

/**
 * @brief SkipNode is a single node resides in the SkipList
 * it has 4-directional links to adjacent nodes
 * @tparam K key type
 * @tparam V value type
 */
template <typename K, typename V>
class SkipNode {
 public:
  /**
   * @brief create a new SkipNode object with the provided key and value
   * @param key key
   * @param value value
   */
  explicit SkipNode(K key, V value, bool is_sentinel = false)
      : key_(key), value_(value), is_sentinel_(is_sentinel) {}

  /**
   * @brief get the key this node is holding
   * @return key
   */
  K GetKey() const { return key_; }

  /**
   * @brief get the value this node is holding
   * @return value
   */
  V GetValue() const { return value_; }

  /**
   * @brief overwrite the existing value by new value
   * @param value the new value to be updated
   */
  void SetValue(V value) { value_ = value; }

  /**
   * @brief give access to the SkipNode before this node
   * @return pointer to SkipNode
   */
  SkipNode *GetBefore() const { return before_; }

  /**
   * @brief give access to the SkipNode after this node
   * @return pointer to SkipNode
   */
  SkipNode *GetAfter() const { return after_; }

  /**
   * @brief give access to the SkipNode below this node
   * @return pointer to SkipNode
   */
  SkipNode *GetBelow() const { return below_; }

  /**
   * @brief give access to the SkipNode above this node
   * @return pointer to SkipNode
   */
  SkipNode *GetAbove() const { return above_; }

  /**
   * @brief if this node's value is a sentinel node
   * @return true if positive infinity, false otherwise
   */
  bool IsSentinel() const { return is_sentinel_; }

  /**
   * @brief set the before node
   * @param node the pointer to SkipNode as the before node
   */
  void SetBefore(SkipNode *node) { before_ = node; }

  /**
   * @brief set the after node
   * @param node the pointer to SkipNode as the after node
   */
  void SetAfter(SkipNode *node) { after_ = node; }

  /**
   * @brief set the below node
   * @param node the pointer to SkipNode as the below node
   */
  void SetBelow(SkipNode *node) { below_ = node; }

  /**
   * @brief set the above node
   * @param node the pointer to SkipNode as the above node
   */
  void SetAbove(SkipNode *node) { above_ = node; }

  /**
   * @brief set this node's value to represent positive infinity
   */
  void SetSentinel() { is_sentinel_ = true; }

  /**
   * @brief starting from this node, search the node with
   *        biggest key that's smaller or equal to the one provided
   * @param key the key for search
   * @return pair of pointer to SkipNode and path of rightmost nodes traversed
   * on each level
   */
  std::pair<SkipNode *, std::vector<SkipNode *>> SkipSearch(K key) const {
    std::vector<SkipNode *> path;
    auto curr = this;
    while (true) {
      while (curr->ShouldSkipRight(key)) {
        curr = curr->GetAfter();
      }
      path.push_back(const_cast<SkipNode *>(curr));
      if (curr->GetBelow() != nullptr) {
        curr = curr->GetBelow();
      } else {
        break;
      }
    }
    return {const_cast<SkipNode *>(curr), path};
  }

 private:
  /**
   * @brief compare with the key, if should proceed going right
   * @param key the provided key
   * @return true if should proceed, false otherwise
   */
  bool ShouldSkipRight(K key) const {
    if (after_ == nullptr || after_->IsSentinel()) {
      return false;
    }
    return after_->GetKey() <= key;
  }
  /** if the node is a sentinel node */
  bool is_sentinel_ = false;
  /** the pointer to the SkipNode before */
  SkipNode *before_ = nullptr;
  /** the pointer to the SkipNode after */
  SkipNode *after_ = nullptr;
  /** the pointer to the SkipNode below */
  SkipNode *below_ = nullptr;
  /** the pointer to the SkipNode above */
  SkipNode *above_ = nullptr;
  /** key for this node */
  K key_;
  /** real value for this node (dominated by neg_inf or inf) */
  V value_;
};

/**
 * @brief SkipList is the backend data structure for key-value store
 *        it uses the SkipNode implemented above as unit of storage
 * @tparam K key type
 * @tparam V value type
 */
template <typename K, typename V>
class SkipList {
 public:
  /**
   * @brief create a new SkipList object
   * @param max_height the maximum height allowed to grow
   */
  explicit SkipList(int max_height = 10) : max_height_(max_height) {
    // create the first layer of sentinel nodes
    head = new SkipNode<K, V>(K{}, V{}, true);
    auto tail = new SkipNode<K, V>(K{}, V{}, true);
    head->SetAfter(tail);
    tail->SetBefore(head);
    curr_height_ = 1;
  }

  /**
   * @brief dtor to release all the SkipNodes in the SkipList
   */
  ~SkipList() {
    // level-by-level dynamic memory release
    auto curr = head;
    while (curr) {
      auto temp = curr->GetBelow();
      ReleaseLevel(curr);
      curr = temp;
    }
  }

  /**
   * @brief search in the SkipList with given key
   * @param key key for search
   * @return the SkipNode with largest key that's smaller or equal to key
   */
  SkipNode<K, V> *SkipSearch(K key) {
      return head->SkipSearch(key).first;
  }

  /**
   * @brief insert into the SkipList of a key-value pair
   * @param key the key
   * @param value the value
   * @return true if insertion is new, false if replace old key-value pair
   */
  bool SkipInsert(K key, V value) {
    std::lock_guard<std::mutex> guard(mutex_);
    auto search_pair = head->SkipSearch(key);
    auto match = search_pair.first;
    auto path = search_pair.second;
    if (match->GetKey() == key && !match->IsSentinel()) {
      // 1. already exist, replace the whole column's value by new value
      while (match) {
        auto temp = match->GetAbove();
        match->SetValue(value);
        match = temp;
      }
      return false;  // indicate a new value replacement for whole column
    } else {
      // 2. a new key-value to be inserted
      int extend_height = rand() % max_height_ + 1;
      if (extend_height > curr_height_) {
        // build new sentinel layer until enough
        int diff = extend_height - curr_height_;
        while (diff--) {
          BuildExtraLayer();
        }
        // re-fetch the search path (not optimal)
        auto research_pair = head->SkipSearch(key);
        match = research_pair.first;
        path = research_pair.second;
      }
      // build the new node all the way up, after each path[i]
      SkipNode<K, V> *last = nullptr;
      for (int i = path.size() - 1; i >= 0; i--) {
        auto pre_node = path[i];
        auto after_node = pre_node->GetAfter();
        auto new_node = new SkipNode<K, V>(key, value);
        pre_node->SetAfter(new_node);
        after_node->SetBefore(new_node);
        new_node->SetBefore(pre_node);
        new_node->SetAfter(after_node);
        new_node->SetBelow(last);
        if (last) {
          last->SetAbove(new_node);
        }
        last = new_node;
      }
      curr_size_++;
      // self-adjust the max height as the SkipList grows
      max_height_ = std::max(max_height_, ExpectedHeight());
      return true;
    }
  }

  /**
   * @brief remove a key from the SkipList
   * @param key the key
   * @return true if removal is successful, false otherwise
   */
  bool SkipRemove(K key) {
    std::lock_guard<std::mutex> guard(mutex_);
    auto match = SkipSearch(key);
    if (match->GetKey() != key || match->IsSentinel()) {
      // not exist in SkipList or is sentinel node
      return false;
    } else {
      // remove the whole column
      auto curr = match;
      while (curr) {
        auto temp = curr->GetAbove();
        auto prev = curr->GetBefore();
        auto next = curr->GetAfter();
        prev->SetAfter(next);
        next->SetBefore(prev);
        delete curr;
        curr = temp;
      }
      return true;
    }
  }

  /**
   * @brief return how many key-value pair are present in the SkipList
   * @return the number of key-value pairs in the SkipList
   */
  std::size_t GetSize() { return curr_size_; }

  /**
   * @brief reassign the max height allowed for this SkipList
   * @param height the new max height allowed
   */
  void SetMaxHeight(int height) { max_height_ = height; }

 private:
  /**
   * @brief build a new layer on top of current head with two sentinel nodes
   */
  void BuildExtraLayer() {
    auto new_head = new SkipNode<K, V>(K{}, V{}, true);
    auto new_tail = new SkipNode<K, V>(K{}, V{}, true);
    head->SetAbove(new_head);
    new_head->SetBelow(head);
    new_head->SetAfter(new_tail);
    new_tail->SetBefore(new_head);

    // find the old top-level's tail
    auto curr = head;
    while (curr->GetAfter() != nullptr) {
      curr = curr->GetAfter();
    }
    new_tail->SetBelow(curr);
    curr->SetAbove(new_tail);

    curr_height_++;
    head = new_head;
  }

  /**
   * @brief in math theory, the expected height of the SkipList is log2(n) + 2
   * @return the expected height of SkipList now
   */
  int ExpectedHeight() const {
      return static_cast<int>(log2(curr_height_)) + 2;
  }

  /**
   * @brief release the whole row of SkipNodes starting from the given one
   * @param node the head of a row of SkipNodes
   */
  void ReleaseLevel(SkipNode<K, V> *node) {
    while (node) {
      auto temp = node->GetAfter();
      delete node;
      node = temp;
    }
  }

  /** the maximum height we allow the SkipList to grow */
  int max_height_;

  /** the current level of the SkipList */
  int curr_height_ = 0;

  /** how many key-value pairs are contained in the SkipList */
  std::size_t curr_size_ = 0;

  /** the mutex for concurrency control */
  std::mutex mutex_;

  /** the top-left sentinel SkipNode in the SkipList */
  SkipNode<K, V> *head = nullptr;
};
}  // namespace kvstore

#endif