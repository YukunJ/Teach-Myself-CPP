#include <algorithm>
#include <chrono>
#include <print>
#include <thread>
#include <stdint.h>
#include <cassert>
#include <vector>
#include <numeric>
#include <pthread.h>
#include <atomic>
#include "lockfree_stack_tagged_pointer.h"

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
    if (argc != 4) {
        std::println("usage: ./executable <producer_num> <consumer_num> <count>");
        exit(1);
    }
    int producer_num = std::stoi(argv[1]);
    int consumer_num = std::stoi(argv[2]);
    int count = std::stoi(argv[3]);
    std::atomic<int> consumed_count{0};
    std::vector<int64_t> each_sum(consumer_num, 0);
    LockfreeStack<int64_t> q;
    std::atomic<bool> begin{false};
    std::vector<int64_t> numbers(count);
    std::iota(numbers.begin(), numbers.end(), 1); 
    std::vector<std::thread> producers;
    for (int i = 0; i < producer_num; i++) {
        producers.emplace_back([&, id=i]() {
            while (!begin.load(std::memory_order_acquire));
            int chunk_size = count / producer_num;
            int begin = id * chunk_size;
            int end = std::min(begin + chunk_size, count);
            for (int i = begin; i < end; i++) {
                q.push(numbers[i]);
            }
        });
    }


    std::vector<std::thread> consumers;
    for (int i = 0; i < consumer_num; i++) {
        consumers.emplace_back([&, id=i]() {
            int64_t sum = 0;
            std::optional<int64_t> ret;
            while (!begin.load(std::memory_order_acquire));
            int chunk_size = count / consumer_num;
            int begin = id * chunk_size;
            int end = (id == consumer_num - 1) ? count : begin + chunk_size;
            for (int i = begin; i < end; ++i) {
                auto ret = q.pop();
                while (!ret.has_value()) {
                    ret = q.pop();
                }
                sum += *ret;
            }
            each_sum[id] = sum;
        });
    }
    auto start = std::chrono::high_resolution_clock::now();
    begin.store(true, std::memory_order_release);
    for (auto &p: producers) {p.join();}
    for (auto &c: consumers) {c.join();}
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    assert(std::accumulate(each_sum.begin(), each_sum.end()) == std::accmulate(numbers.begin(), numbers.end()));
    std::println("benchmark finishes");
    std::println("throughput {} ops/s", count * 1000.0 /ms);
    return 0;
}