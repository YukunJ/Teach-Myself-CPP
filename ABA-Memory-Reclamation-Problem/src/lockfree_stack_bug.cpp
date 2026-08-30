// This version has an ABA vulnerability and does not yet have a memory reclamation strategy
#include <stdint.h>
#include "lockfree_stack.h"

template<typename T>
void LockfreeStack<T>::push(const T& value) {
    Node *new_node = new Node(value);
    Node *old_head = head_.load(std::memory_order_relaxed);
    // the following compare_exchange_weak has ABA problems
    // another thread might have popped out 2 nodes and insert a new node, which happens to reuse the memory address of old_head
    do {
        new_node->next_ = old_head;
    } while(!head_.compare_exchange_weak(old_head, new_node, std::memory_order_release, std::memory_order_relaxed));
}

template<typename T>
std::optional<T> LockfreeStack<T>::pop(void) {
    Node *old_head = head_.load(std::memory_order_acquire);
    while (old_head) {
        Node *new_head = old_head->next_;
        if (head_.compare_exchange_weak(old_head, new_head, std::memory_order_acquire, std::memory_order_relaxed)) {
            T to_ret = std::move(old_head->value_);
            // cannot delete old_head here in case other thread is holding a reference to it
            return to_ret;
        }
    }
    return std::nullopt;
}

template class LockfreeStack<int64_t>;