#define _POSIX_C_SOURCE 200809L
#include "spsc_queue.hpp"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  unsigned char buf[100];
  auto result = SpscQueue::create("/spsc_test_queue", 100, 16, SpscMode::Writer);
  if (!result) {
    fprintf(stderr, "Failed to create SpscQueue: %d\n", static_cast<int>(result.error()));
    return 1;
  }
  auto queue = std::move(result.value());
  sprintf((char *)buf, "Hello from spmc_queue producer!\n");
  int counter = 0;
  while (true) {
    bool enqueue = queue->try_enqueue(buf);
    counter += (int)enqueue;
    if (counter == 1024) {
      break;
    }
  }
  printf("producer enqueued 1024 messages into the queue\n");
  return 0;
}