// This version create a serial tag number on the head update to solve ABA problem
// but does not yet have a memory reclamation strategy
#include <stdint.h>
#include "lockfree_stack_tagged_pointer.h"

template<typename T>
void LockfreeStack<T>::push(const T& value) {
    Node *new_node = new Node(value);
    TaggedPointer old_head = head_.load(std::memory_order_relaxed);
    // tagged pointer resolves the ABA problem in the following CAS
    // relies on the availability of 128 bit CAS instruction 

    // assume original the stack is like B -> C
    // thread 1 about to push A and set it to be A -> B -> C
    // thread 2 pop out B and push in a new B' to be B' -> C
    // need to make sure thread 1 realize B != B' even if the same address
    do {
        new_node->next_ = old_head.node_;
    } while(!head_.compare_exchange_weak(old_head, TaggedPointer(new_node, old_head.tag_+1), std::memory_order_release, std::memory_order_relaxed));
}

template<typename T>
std::optional<T> LockfreeStack<T>::pop(void) {
    TaggedPointer old_head = head_.load(std::memory_order_acquire);
    while (old_head.node_) {
        Node *next_node = old_head.node_->next_;
        if (head_.compare_exchange_weak(old_head, TaggedPointer(next_node, old_head.tag_+1), std::memory_order_acquire, std::memory_order_relaxed)) {
            T to_ret = std::move(old_head.node_->value_);
            // still cannot delete old_head here in case other thread is holding a reference to it
            return to_ret;
        }
    }
    return std::nullopt;
}

template class LockfreeStack<int64_t>;