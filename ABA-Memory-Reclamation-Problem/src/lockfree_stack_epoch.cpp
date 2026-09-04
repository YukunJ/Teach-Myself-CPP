// This version use epoch-based memory reclamation to solve both the ABA and memory reclamation problems
#include <stdint.h>
#include "lockfree_stack_epoch.h"

template<typename T>
void LockfreeStack<T>::push(const T& value, size_t thread_id) {
    // typename LockfreeStack<T>::EpochManager::Guard guard(epoch_manager_, thread_id);
    Node *new_node = new Node(value);
    Node *old_head = head_.load(std::memory_order_relaxed);
    do {
        new_node->next_ = old_head;
    } while(!head_.compare_exchange_weak(old_head, new_node, std::memory_order_release, std::memory_order_relaxed));
}

template<typename T>
std::optional<T> LockfreeStack<T>::pop(size_t thread_id) {
    typename LockfreeStack<T>::EpochManager::Guard guard(epoch_manager_, thread_id);
    Node *old_head = head_.load(std::memory_order_seq_cst);
    while (old_head) {
        Node *new_head = old_head->next_;
        // Either thread A here sees thread B epoch publication, or B must see thread A' removal of the node before B dereferences it.
        if (head_.compare_exchange_weak(old_head, new_head, std::memory_order_seq_cst, std::memory_order_acquire)) {
            T to_ret = std::move(old_head->value_);
            epoch_manager_.retire_node(old_head);
            return to_ret;
        }
    }
    return std::nullopt;
}

template class LockfreeStack<int64_t>;

template<typename T>
thread_local std::vector<
    std::pair<uint64_t, typename LockfreeStack<T>::Node*>
> LockfreeStack<T>::EpochManager::retired_nodes_;