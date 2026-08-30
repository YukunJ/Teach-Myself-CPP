#pragma once
#include <atomic>
#include <optional>
#include <print>
#include <type_traits>
template<typename T>
class LockfreeStack {
  public:
    struct Node {
        T value_;
        Node* next_;
        explicit Node(const T& value): value_(value), next_(nullptr) {}
    };

    struct alignas(16) TaggedPointer {
      Node *node_;
      int64_t tag_;
      explicit TaggedPointer(Node *node, int64_t tag = 0): node_(node), tag_(tag) {}
    };
    static_assert(sizeof(TaggedPointer) == 16 && "Should be == 128 bits to utilize hardware 128-bit CAS");
    static_assert(std::is_trivially_copyable_v<TaggedPointer> == true);
    LockfreeStack(): head_(TaggedPointer(nullptr, 0)) {}
    void push(const T& value);
    std::optional<T> pop(void);

  private:
    std::atomic<TaggedPointer> head_ = nullptr;
};