#pragma once
#include <atomic>
#include <optional>
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
    class EpochManager {
        friend class LockfreeStack<T>;
        struct Guard {
            Guard(EpochManager& manager, uint64_t thread_id): manager_(manager), thread_id_(thread_id) {
                while (true) {
                    epoch_ = manager_.global_epoch_.load(std::memory_order_seq_cst);
                    manager_.thread_epochs_[thread_id_].epoch_.store(epoch_, std::memory_order_seq_cst);
                    // validation
                    if (epoch_ == manager_.global_epoch_.load(std::memory_order_seq_cst)) {
                        break;
                    }
                }
            }

            ~Guard() noexcept {
                manager_.thread_epochs_[thread_id_].epoch_.store(UINT64_MAX, std::memory_order_seq_cst);
            }
            EpochManager& manager_;
            uint64_t thread_id_;
            uint64_t epoch_;
        };

        // method   
        void retire_node(Node* node) {
            retired_nodes_.push_back({global_epoch_.load(std::memory_order_seq_cst), node});
            if (retired_nodes_.size() >= kMaxThreads) {
                // find the current global epoch and advance it
                global_epoch_.fetch_add(1, std::memory_order_seq_cst);
                // find min_epoch across threads
                uint64_t min_epoch = UINT64_MAX;
                for (size_t i = 0; i < kMaxThreads; ++i) {
                    uint64_t thread_epoch = thread_epochs_[i].epoch_.load(std::memory_order_seq_cst);
                    if (thread_epoch < min_epoch) {
                        min_epoch = thread_epoch;
                    }
                }
                // retire any old epoch nodes
                std::erase_if(retired_nodes_, [min_epoch](const auto& p) {
                    if (p.first < min_epoch) {
                        delete p.second;
                        return true;
                    }
                    return false;
                });
            }
        }

        // data
        struct alignas(64) ThreadEpoch {
            std::atomic<uint64_t> epoch_{UINT64_MAX};
        } thread_epochs_[kMaxThreads];
        std::atomic<uint64_t> global_epoch_{2};
        thread_local static std::vector<std::pair<uint64_t, Node*>> retired_nodes_;
    } epoch_manager_;

    std::atomic<Node*> head_ = nullptr;
};