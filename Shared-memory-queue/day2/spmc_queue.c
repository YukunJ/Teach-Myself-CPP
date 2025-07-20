#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "spmc_queue.h"

spmc_queue_t *spmc_queue_create(const char *path, size_t element_size, size_t element_capacity, enum spmc_mode mode) {
    if (!path || element_size == 0 || element_capacity == 0 || (mode != spmc_mode_reader && mode != spmc_mode_writer)) {
        fprintf(stderr, "spmc_queue_create: invalid arguments (path:%s element_size:%zu element_capacity:%zu mode:%d\n", path, element_size, element_capacity, mode);
        return NULL;
    }
    printf("creating spmc_queue %s of size %zu and capacity %zu with mode %s\n", path, element_size, element_capacity, (mode == spmc_mode_reader) ? "reader" : "writer");
    spmc_queue_t *queue = NULL;
    spmc_shared_t *shared = NULL;
    int fd = -1;
    size_t shared_size = sizeof(spmc_shared_t) + element_size * element_capacity;
    int oflag = (mode == spmc_mode_reader) ? O_RDWR : O_RDWR | O_CREAT | O_EXCL;

    fd = shm_open(path, oflag, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) { perror("shm_open"); goto cleanup; }
    if (mode == spmc_mode_writer) {
        int rt = ftruncate(fd, shared_size);
        if (rt == -1) { perror("ftruncate"); goto cleanup; }
    }
    queue = calloc(1, sizeof(spmc_queue_t));
    if (!queue) { perror("calloc(1, sizeof(spmc_queue_t))"); goto cleanup;}
    shared = mmap(NULL, shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) { perror("mmap"); goto cleanup; }
    queue->shared = shared;

    queue->header.fd = fd;
    queue->header.mode = mode;
    queue->header.path = strdup(path); if (!queue->header.path) { perror("strdup"); goto cleanup; }
    queue->header.shared_size = shared_size;
    queue->shared->version = SPMC_QUEUE_VERSION;
    queue->shared->element_size = element_size;
    queue->shared->element_capacity = element_capacity;

    if (mode == spmc_mode_writer) {
        queue->shared->initialized = true;
        queue->shared->client_connected = false;
        queue->shared->writer_idx = 0;
    }
    if (mode == spmc_mode_reader) {
        if (!queue->shared->initialized) {
            fprintf(stderr, "spmc_queue: reader mode encounters shared memory not initialized yet by writer\n");
            goto cleanup;
        }
        queue->shared->client_connected = true;
        queue->shared->reader_idx = 0;
    }
    printf("spmc_queue created\n");
    return queue;

cleanup:
    if (fd != -1) close(fd);
    if (shared && shared != MAP_FAILED) munmap(shared, shared_size);
    if (mode == spmc_mode_writer && path) shm_unlink(path);
    if (queue && queue->header.path) free(queue->header.path);
    if (queue) free(queue);
    return NULL;
}

void spmc_queue_destroy(spmc_queue_t *queue) {
  printf("destroying spmc_queue %s of mode %s\n", queue->header.path, (queue->header.mode == spmc_mode_reader) ? "reader" : "writer");
  int fd = queue->header.fd;
  char *path = queue->header.path;
  enum spmc_mode mode = queue->header.mode;
  munmap(queue->shared, queue->header.shared_size);
  close(fd);
  if (mode == spmc_mode_writer) {
    // writer owns the lifecycle of the queue
    shm_unlink(path);
  }
  free(path);
  free(queue);
  printf("spmc_queue destroyed\n");
}

bool spmc_queue_enqueue(spmc_queue_t *queue, uint8_t *src_data) {
  if (!queue->shared->client_connected) {
    return false;
  }
  assert(queue->shared->writer_idx < queue->shared->element_capacity);
  memcpy(&queue->shared->data[queue->shared->writer_idx * queue->shared->element_size], src_data, queue->shared->element_size);
  queue->shared->writer_idx++;
  return true;
}

bool spmc_queue_dequeue(spmc_queue_t *queue, uint8_t *dst_data) {
  if (queue->shared->reader_idx >= queue->shared->writer_idx) {
    return false;
  }
  memcpy(dst_data, &queue->shared->data[queue->shared->reader_idx * queue->shared->element_size], queue->shared->element_size);
  queue->shared->reader_idx++;
  return true;
}