#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void) {
  printf("Consumer attaching to the shared-memory queue /queue...\n");
  const size_t queue_size = 100;
  char buf[queue_size];
  int fd = shm_open("/queue", O_RDWR, 0600);
  assert(ftruncate(fd, queue_size) == 0);
  void *queue = mmap(NULL, queue_size, PROT_READ, MAP_SHARED, fd, 0);
  printf("Consumer attached to the shared-memory queue /queue.\n");
  memcpy(buf, queue, strlen(queue));
  printf("Consumer read a msg from queue: \"%s\"\n", buf);
  munmap(queue, queue_size);
  close(fd);
  shm_unlink("/queue");
  printf("Consumer closed the queue and exit...\n");
  return 0;
}