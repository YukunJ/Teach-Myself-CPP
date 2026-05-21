#define _POSIX_C_SOURCE 200809L
#include "spsc_queue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  unsigned char buf[100];
  memset(buf, 0, 100);
  spsc_queue_t *queue = spsc_queue_create("/spsc_test_queue", 100, 16, spsc_mode_reader);
  int counter = 0;
  while (true) {
    bool dequeue = spsc_queue_dequeue(queue, buf);
    counter += (int)dequeue;
    if (counter == 1024) {
      break;
    }
  }
  printf("Received %d message from producer: %s", counter, buf);
  spsc_queue_destroy(queue);
  return 0;
}