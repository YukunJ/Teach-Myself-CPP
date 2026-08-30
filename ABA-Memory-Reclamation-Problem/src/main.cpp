#include <algorithm>
#include <chrono>
#include <print>
#include <thread>
#include <stdint.h>
#include <cassert>
#include <vector>
#include <numeric>
#include <pthread.h>
#include "lockfree_stack.h"

static constexpr int count = 1'000'000;

static void pin_to_core(int core_id) {
    pthread_t hp = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    
    CPU_SET(core_id, &cpuset);

    int rc = pthread_setaffinity_np(hp, sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        std::println("Error pin thread to core {}", core_id);
    } else {
        std::println("Successfully pin thread to core {}", core_id);
    }
}

int main(int argc, const char *argv[]) {
    LockfreeStack<int64_t> q;
    std::atomic<bool> begin{false};
    std::vector<int64_t> numbers(count);
    std::iota(numbers.begin(), numbers.end(), 1); 
    std::thread producer([&]() {
        pin_to_core(1);
        while (!begin.load(std::memory_order_acquire));
        for (auto v: numbers) {
            q.push(v);
        }
    });

    std::thread consumer([&]() {
        pin_to_core(2);
        int64_t sum = 0;
        std::optional<int64_t> ret;
        while (!begin.load(std::memory_order_acquire));
        for (int i = 0; i < numbers.size(); i++) {
            ret = q.pop();
            while (!ret.has_value()) {
                ret = q.pop();
            }
            sum += ret.value();
        }
    });
    auto start = std::chrono::high_resolution_clock::now();
    begin.store(true, std::memory_order_release);
    producer.join();
    consumer.join();
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    assert(sum == std::accmulate(numbers.begin(), numbers.end()));
    std::println("benchmark finishes");
    std::println("throughput {} ops/s", count * 1000.0 /ms);
    return 0;
}