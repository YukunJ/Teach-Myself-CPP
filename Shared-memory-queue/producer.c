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
  printf("Producer creating the shared-memory queue /queue...\n");
  size_t queue_size = 100;
  const char *msg = "Hello from producer";
  int fd = shm_open("/queue", O_RDWR | O_CREAT | O_EXCL, 0600);
  assert(ftruncate(fd, queue_size) == 0);
  void *queue = mmap(NULL, queue_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  printf("Producer created the shared-memory queue /queue of size %ld and mapped it into memory.\n", queue_size);
  memcpy(queue, msg, strlen(msg));
  printf("Producer sent a msg \"%s\" into the queue.\n", msg);
  sleep(5);
  munmap(queue, queue_size);
  close(fd);
  shm_unlink("/queue");
  printf("Producer closed the queue after 5 seconds and exit...\n");
  return 0;
}