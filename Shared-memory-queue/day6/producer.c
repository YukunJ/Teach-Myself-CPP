#define _POSIX_C_SOURCE 200809L
#include "spsc_queue.hpp"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  unsigned char buf[100];
  spsc_queue_t *queue = spsc_queue_create("/spsc_test_queue", 100, 16, spsc_mode_writer);
  sprintf((char *)buf, "Hello from spmc_queue producer!\n");
  int counter = 0;
  while (true) {
    bool enqueue = spsc_queue_enqueue(queue, buf);
    counter += (int)enqueue;
    if (counter == 1024) {
      break;
    }
  }
  spsc_queue_destroy(queue);
  printf("producer enqueued 1024 messages into the queue\n");
  return 0;
}