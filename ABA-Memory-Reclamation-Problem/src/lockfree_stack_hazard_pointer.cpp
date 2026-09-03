// This hazard pointer version solves the ABA problem and also has a memory reclamation strategy 
#include <stdint.h>
#include "lockfree_stack_hazard_pointer.h"

template<typename T>
void LockfreeStack<T>::push(const T& value, size_t thread_id) {
    Node *new_node = new Node(value);
    Node *old_head = head_.load(std::memory_order_acquire);
    // it's not needed to set hazard pointer here since we are not going to read/refernce the old head node
    // even if another thread pop and reallocate a head using the same address of the old head
    // we can still just take the new head and link to it
    do {
        new_node->next_ = old_head;
    } while (!head_.compare_exchange_weak(old_head, new_node, std::memory_order_release, std::memory_order_relaxed));
}

template<typename T>
std::optional<T> LockfreeStack<T>::pop(size_t thread_id) {
    Node *old_head;

    while (true) {
        old_head = head_.load(std::memory_order_acquire);
        if (!old_head) {
            return std::nullopt;
        }
        hazard_pointers_[thread_id].ptr_.store(old_head, std::memory_order_seq_cst);
        if (old_head != head_.load(std::memory_order_seq_cst)) {
            // ensure between we load the head and publish the hazard pointer, the head has not changed
            continue;
        }

        Node *next_node = old_head->next_;
        // important to use seq_cst here for the success publish case
        // to let the unlink participate in the global ordering of
        // hazard pointer publication and re-verify 
        if (head_.compare_exchange_weak(old_head, next_node, std::memory_order_seq_cst, std::memory_order_relaxed)) {
            T to_ret = std::move(old_head->value_);
            hazard_pointers_[thread_id].ptr_.store(nullptr, std::memory_order_seq_cst);
            retire_node(old_head);
            return to_ret;
        }
        // don't bother to clean up the hazard pointer here since we will overwrite it in the next iteration anyway
    }
}

template class LockfreeStack<int64_t>;

template<typename T>
thread_local std::vector<typename LockfreeStack<T>::Node*> LockfreeStack<T>::retired_nodes_;