#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spmc_queue.h"

int main(void) {
  unsigned char buf[100];
  spmc_queue_t *queue = spmc_queue_create("/spmc_test_queue", 100, 10, spmc_mode_writer);
  sprintf((char *)buf, "Hello from spmc_queue producer!\n");
  int counter = 0;
  while (true) {
    bool enqueue = spmc_queue_enqueue(queue, buf);
    counter += (int)enqueue;
    if (counter == 1024) {
      break;
    }
  }
  spmc_queue_destroy(queue);
  printf("producer enqueued 1024 messages into the queue\n");
  return 0;
}