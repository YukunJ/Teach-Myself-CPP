#pragma once
#include <atomic>
#include <optional>
#include <print>
#include <type_traits>
#include <vector>

constexpr inline static size_t kMaxThreads = 16;
constexpr inline static size_t kCacheLineSize = 64;

template<typename T>
class LockfreeStack {
  public:
    struct Node {
        T value_;
        Node* next_;
        explicit Node(const T& value): value_(value), next_(nullptr) {}
    };

    LockfreeStack(): head_(nullptr) {}
    void push(const T& value, size_t thread_id);
    std::optional<T> pop(size_t thread_id);

  private:
    void retire_node(Node* node) {
      retired_nodes_.push_back(node);
      if (retired_nodes_.size() >= kMaxThreads) {
        std::vector<Node*> new_retired_nodes;
        new_retired_nodes.reserve(kMaxThreads);
        for (Node* node: retired_nodes_) {
          bool can_delete = true;
          for (size_t i = 0; i < kMaxThreads; ++i) {
            if (hazard_pointers_[i].ptr_.load(std::memory_order_acquire) == node) {
              can_delete = false;
              break;
            }
          }
          if (can_delete) {
            delete node;
          } else {
            new_retired_nodes.push_back(node);
          }
        }
        retired_nodes_.swap(new_retired_nodes);
      }
    }

    void publish_hazard(Node* node, size_t thread_id) {
      hazard_pointers_[thread_id].ptr_.store(node, std::memory_order_release);
    }

    std::atomic<Node*> head_ = nullptr;
    struct alignas(kCacheLineSize) HazardPointer { std::atomic<Node*> ptr_{nullptr};} hazard_pointers_[kMaxThreads];
    static thread_local std::vector<Node*> retired_nodes_;
};