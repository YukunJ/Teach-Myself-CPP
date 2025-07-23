#define _POSIX_C_SOURCE 200809L
#include "spmc_queue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  unsigned char buf[100];
  memset(buf, 0, 100);
  spmc_queue_t *queue = spmc_queue_create("/spmc_test_queue", 100, 10, spmc_mode_reader);
  int counter = 0;
  while (true) {
    bool dequeue = spmc_queue_dequeue(queue, buf);
    counter += (int)dequeue;
    if (counter == 1024) {
      break;
    }
  }
  printf("Received %d message from producer: %s", counter, buf);
  spmc_queue_destroy(queue);
  return 0;
}