/**
 * skiplist.h
 * This is the implementation of a simple SkipList data structure using linked list
 * It supports three main APIs: SkipSearch, SkipInsert and SkipRemove
 * We use linked node of 4-directional links to connect the list and maintain the top-left header
*/
#ifndef KVSTORE_SKIPLIST_H
#define KVSTORE_SKIPLIST_H

namespace kvstore {

/**
 * @brief SkipNode is a single node resides in the SkipList
 * it has 4-directional links to adjacent nodes
 * @tparam K key type
 * @tparam V value type
 */
template<typename K, typename V>
class SkipNode {
public:
    /**
     * @brief create a new SkipNode object with the provided key and value
     * @param key key
     * @param value value
     */
    explicit SkipNode(K key, V value, bool is_sentinel = false): key_(key), value_(value), is_sentinel_(is_sentinel) {}

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
     * @return pointer to SkipNode
     */
    SkipNode *SkipSearch(K key) const {
        auto curr = this;
        while (true) {
            while (curr->ShouldSkipRight(key)) {
                curr = curr->GetAfter();
            }
            if (curr->GetBelow() != nullptr) {
                curr = curr->GetBelow();
            } else {
                break;
            }
        }
        return const_cast<SkipNode*>(curr);
    }

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

private:
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
template<typename K, typename V>
class SkipList {
};
} // namespace kvstore

#endif