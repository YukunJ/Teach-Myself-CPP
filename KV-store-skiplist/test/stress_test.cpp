/**
 * Performance testing for the SkipList
 * on random query and insert
 * using std::thread
 */

#include "../src/skiplist.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>
#include <vector>
#include <assert.h>

kvstore::SkipList<int, int> test_list;

/**
 * @brief insertion test on a thread
 *        the insert range is based on how many threads are there in total
 * @param thread_id the thread's id
 * @param thread_num how many threads are there in total
 * @param test_load the total number of test load to be done
 */
void *insertTest(long thread_id, long thread_num, long test_load) {
    std::cout << "launch insertTest with thread " << thread_id+1 << "/" << thread_num << std::endl;
    for (size_t i = thread_id; i < test_load; i += thread_num) {
        test_list.SkipInsert(i, i);
    }
    return 0;
}


int main(int argc, const char *argv[]) {
    // usage: ./stress_test [number of threads] [number of test load] [max_height of the SkipList]
    assert(argc == 4 && "usage: ./stress_test [number of threads] [number of test load] [max_height of the SkipList]");
    long num_thread, test_load, max_height;
    num_thread = strtol(argv[1], nullptr, 10);
    test_load = strtol(argv[2], nullptr, 10);
    max_height = strtol(argv[3], nullptr, 10);

    test_list.SetMaxHeight(max_height);

    std::cout << "--------Test Spec--------" << std::endl;
    std::cout << "Launch test of load " << test_load << std::endl;
    std::cout << "with " << num_thread << " threads" << std::endl;
    std::cout << "max height of the SkipList is set to " << max_height << std::endl;
    std::cout << "---------------------------" << std::endl;

    {
        std::cout << "--------Insertion Test--------" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        std::vector <std::thread> threads;
        for (long i = 0; i < num_thread; i++) {
            threads.emplace_back(insertTest, i, num_thread, test_load);
        }
        for (auto &thr: threads) {
            thr.join();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Insertion Test takes " << std::setw(6) << elapsed.count() << "s" << std::endl;
        std::cout << "Throughput is " << static_cast<int>(static_cast<double>(test_load) / elapsed.count()) << std::endl;
    }

    return 0;
}