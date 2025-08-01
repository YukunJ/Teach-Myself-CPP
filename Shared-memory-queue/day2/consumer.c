#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spmc_queue.h"

int main(void) {
  unsigned char buf[100];
  memset(buf, 0, 100);
  spmc_queue_t *queue = spmc_queue_create("/spmc_test_queue", 100, 10, spmc_mode_reader);
  while (!spmc_queue_dequeue(queue, buf)) {}
  printf("Received message from producer: %s", buf);
  spmc_queue_destroy(queue);
  return 0;
}