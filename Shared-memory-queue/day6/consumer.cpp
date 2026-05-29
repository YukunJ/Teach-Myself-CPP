#define _POSIX_C_SOURCE 200809L
#include "spsc_queue.hpp"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  unsigned char buf[100];
  memset(buf, 0, 100);
  auto result = SpscQueue::create("/spsc_test_queue", 100, 16, SpscMode::Reader);
  if (!result) {
    fprintf(stderr, "Failed to create SpscQueue: %d\n", static_cast<int>(result.error()));
    return 1;
  }
  auto queue = std::move(result.value());
  int counter = 0;
  while (true) {
    bool dequeue = queue->try_dequeue(buf);
    counter += (int)dequeue;
    if (counter == 1024) {
      break;
    }
  }
  printf("Received %d message from producer: %s", counter, buf);
  return 0;
}