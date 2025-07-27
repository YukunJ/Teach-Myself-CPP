#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include "spmc_queue.h"
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define UNUSED(arg) ((void)arg)
#define TEST_MESSAGE_COUNT 1024 * 1024 * 64 // 64 MB count * 64 bytes = 4 GB data

#define QUEUE_CAPACITY 1024
struct message {
  int64_t num;
  char padding[L1_DCACHE_LINESIZE - sizeof(int64_t)];
};

static spmc_queue_t *producer_queue = NULL;
static spmc_queue_t *consumer_queue = NULL;
static pthread_t producer_thread;
static pthread_t consumer_thread;
static struct message *test_messages;

static volatile bool producer_thread_ready = false;
static volatile bool consumer_thread_ready = false;
static volatile bool test_may_start = false;
static int64_t test_producer_sum = 0;
static int64_t test_consumer_sum = 0;

static void initialize_benchmark(void) {
  printf("Initializing the performance benchmark...\n");
  producer_thread_ready = false;
  consumer_thread_ready = false;
  test_may_start = false;
  test_producer_sum = 0;
  test_consumer_sum = 0;
  producer_queue = spmc_queue_create("/spmc_benchmark_queue", sizeof(struct message), QUEUE_CAPACITY, spmc_mode_writer);
  consumer_queue = spmc_queue_create("/spmc_benchmark_queue", sizeof(struct message), QUEUE_CAPACITY, spmc_mode_reader);
  assert(producer_queue != NULL);
  assert(consumer_queue != NULL);
  test_messages = calloc(TEST_MESSAGE_COUNT, sizeof(struct message));
  for (int i = 0; i < TEST_MESSAGE_COUNT; i++) {
    int random_number = rand() % 5;
    test_messages[i].num = random_number;
    test_producer_sum += random_number;
  }
  printf("Initialized performance benchmark\n");
}

static void pin_to_core(int core_num) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_num, &cpuset);
  pthread_t current_thread = pthread_self();
  int rc = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
  assert(rc == 0);
}

static void destroy_benchmark(void) {
  printf("Destroying the performance benchmark...\n");
  spmc_queue_destroy(producer_queue);
  spmc_queue_destroy(consumer_queue);
  producer_queue = NULL;
  consumer_queue = NULL;
  free(test_messages);
  printf("Destroyed performance benchmark\n");
}

static void *consumer_main(void *arg) {
  UNUSED(arg);
  pin_to_core(5);
  printf("consumer thread spawns\n");
  consumer_thread_ready = true;
  static struct message message_buf;
  while (!test_may_start) {
  }
  int idx = 0;
  while (idx < TEST_MESSAGE_COUNT) {
    bool dequeued = spmc_queue_dequeue(consumer_queue, (unsigned char *)&message_buf);
    if (dequeued) {
      idx++;
      test_consumer_sum += message_buf.num;
    }
  }
  return NULL;
}

static void *producer_main(void *arg) {
  UNUSED(arg);
  pin_to_core(7);
  printf("producer thread spawns\n");
  producer_thread_ready = true;
  while (!test_may_start) {
  }
  int idx = 0;
  while (idx < TEST_MESSAGE_COUNT) {
    idx += (int)spmc_queue_enqueue(consumer_queue, (unsigned char *)&test_messages[idx]);
  }
  return NULL;
}

int main(void) {
  struct timespec start;
  struct timespec end;
  double elapsed_sec;
  double total_bytes = (double)TEST_MESSAGE_COUNT * sizeof(struct message);
  double throughput_mb;

  initialize_benchmark();
  pthread_create(&producer_thread, NULL, producer_main, NULL);
  pthread_create(&consumer_thread, NULL, consumer_main, NULL);
  while (!producer_thread_ready || !consumer_thread_ready) {
    static bool prompt = false;
    if (!prompt) {
      printf("waiting for the producer & consumer thread to be ready...\n");
      prompt = true;
    }
  }
  clock_gettime(CLOCK_MONOTONIC, &start);
  test_may_start = true;
  printf("performance benchmark starts\n");
  pthread_join(producer_thread, NULL);
  pthread_join(consumer_thread, NULL);
  clock_gettime(CLOCK_MONOTONIC, &end);
  printf("performance benchmark ends\n");

  elapsed_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
  throughput_mb = total_bytes / elapsed_sec / (1024 * 1024);
  printf("Elapsed time: %.3f seconds\n", elapsed_sec);
  printf("Throughput: %.3f MB/s\n", throughput_mb);
  printf("test_producer_sum = %ld and test_consumer_sum = %ld\n", test_producer_sum, test_consumer_sum);
  assert(test_producer_sum == test_consumer_sum);
  destroy_benchmark();
  return 0;
}