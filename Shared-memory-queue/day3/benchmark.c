#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "spmc_queue.h"

#define L1_DCACHE_LINESIZE 64 // found via /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size

#define TEST_MESSAGE_COUNT 1024 * 1024 * 16 // 16 MB count * 64 bytes = 1 GB data 

#define QUEUE_CAPACITY 1024
struct message {
    int64_t num;
    char padding[L1_DCACHE_LINESIZE-sizeof(int64_t)];
};

static spmc_queue_t *producer_queue = NULL;
static spmc_queue_t *consumer_queue = NULL;
static struct message test_messages[TEST_MESSAGE_COUNT];

static bool test_may_start = false;
static int64_t test_producer_sum = 0;
static int64_t test_consumer_sum = 0;

static void initialize_benchmark(void) {
    printf("Initializing the performance benchmark...\n");
    test_may_start = false;
    test_producer_sum = 0;
    test_consumer_sum = 0;
    producer_queue = spmc_queue_create("/spmc_benchmark_queue", sizeof(struct message), QUEUE_CAPACITY, spmc_mode_writer);
    consumer_queue = spmc_queue_create("/spmc_benchmark_queue", sizeof(struct message), QUEUE_CAPACITY, spmc_mode_reader);
    assert(producer_queue != NULL);
    assert(consumer_queue != NULL);
    memset(test_messages, 0, sizeof(test_messages));
    for (int i = 0; i < TEST_MESSAGE_COUNT; i++) {
        int random_number = rand() % 5;
        test_messages[i].num = random_number;
        test_producer_sum += random_number;
    }
    printf("Initialized performance benchmark\n");
}

static void destroy_benchmark(void) {
    printf("Destroying the performance benchmark...\n");
    spmc_queue_destroy(producer_queue);
    spmc_queue_destroy(consumer_queue);
    producer_queue = NULL;
    consumer_queue = NULL;
    printf("Destroyed performance benchmark\n");
}

int main(void) {
  initialize_benchmark();
  destroy_benchmark();
  return 0;
}