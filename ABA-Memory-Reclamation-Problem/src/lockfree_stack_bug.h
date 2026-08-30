#pragma once
#include <atomic>
#include <optional>

template<typename T>
class LockfreeStack {
  public:
    struct Node {
        T value_;
        Node* next_;
        explicit Node(const T& value): value_(value), next_(nullptr) {}
    };
    LockfreeStack(): head_(nullptr) {}
    void push(const T& value);
    std::optional<T> pop(void);

  private:
    std::atomic<Node*> head_ = nullptr;
};