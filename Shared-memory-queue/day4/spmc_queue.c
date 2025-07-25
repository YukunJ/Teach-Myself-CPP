#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "spmc_queue.h"

spmc_queue_t *spmc_queue_create(const char *path, size_t element_size, size_t element_capacity, enum spmc_mode mode) {
  if (!path || element_size == 0 || element_capacity == 0 || (mode != spmc_mode_reader && mode != spmc_mode_writer)) {
    fprintf(stderr, "spmc_queue_create: invalid arguments (path:%s element_size:%zu element_capacity:%zu mode:%d\n",
            path, element_size, element_capacity, mode);
    return NULL;
  }
  printf("creating spmc_queue %s of size %zu and capacity %zu with mode %s\n", path, element_size, element_capacity,
         (mode == spmc_mode_reader) ? "reader" : "writer");
  spmc_queue_t *queue = NULL;
  spmc_shared_t *shared = NULL;
  int fd = -1;
  size_t shared_size = sizeof(spmc_shared_t) + element_size * element_capacity;
  int oflag = (mode == spmc_mode_reader) ? O_RDWR : O_RDWR | O_CREAT | O_EXCL;

  fd = shm_open(path, oflag, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd == -1) {
    perror("shm_open");
    goto cleanup;
  }
  if (mode == spmc_mode_writer) {
    int rt = ftruncate(fd, shared_size);
    if (rt == -1) {
      perror("ftruncate");
      goto cleanup;
    }
  }
  queue = calloc(1, sizeof(spmc_queue_t));
  if (!queue) {
    perror("calloc(1, sizeof(spmc_queue_t))");
    goto cleanup;
  }
  shared = mmap(NULL, shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shared == MAP_FAILED) {
    perror("mmap");
    goto cleanup;
  }

  queue->header.fd = fd;
  queue->header.mode = mode;
  queue->header.path = strdup(path);
  if (!queue->header.path) {
    perror("strdup");
    goto cleanup;
  }
  queue->header.shared_size = shared_size;
  queue->shared = shared;

  if (mode == spmc_mode_writer) {
    memset(shared, 0, shared_size);
    queue->shared->version = SPMC_QUEUE_VERSION;
    queue->shared->initialized = true;
    queue->shared->client_connected = false;
    queue->shared->element_size = element_size;
    queue->shared->element_capacity = element_capacity;
    queue->shared->writer_idx = 0;
  }
  if (mode == spmc_mode_reader) {
    if (!queue->shared->initialized) {
      fprintf(stderr, "spmc_queue: reader mode encounters shared memory not initialized yet by writer\n");
      goto cleanup;
    }
    if (queue->shared->element_capacity != element_capacity) {
      fprintf(stderr, "spmc_queue: element_capacity %zu != queue's element_capacity %zu\n",
              queue->shared->element_capacity, element_capacity);
      goto cleanup;
    }
    if (queue->shared->element_size < element_size) {
      fprintf(stderr, "spmc_queue: element size %zu > queue's element size %zu\n", element_size,
              queue->shared->element_size);
      goto cleanup;
    }
    queue->shared->client_connected = true;
    queue->shared->reader_idx = 0;
  }
  printf("spmc_queue created\n");
  return queue;

cleanup:
  if (fd != -1)
    close(fd);
  if (shared && shared != MAP_FAILED)
    munmap(shared, shared_size);
  if (mode == spmc_mode_writer && path)
    shm_unlink(path);
  if (queue && queue->header.path)
    free(queue->header.path);
  if (queue)
    free(queue);
  return NULL;
}

void spmc_queue_destroy(spmc_queue_t *queue) {
  printf("destroying spmc_queue %s of mode %s\n", queue->header.path,
         (queue->header.mode == spmc_mode_reader) ? "reader" : "writer");
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
  int64_t reader_idx = atomic_load_explicit(&queue->shared->reader_idx, memory_order_acquire);
  int64_t writer_idx = atomic_load_explicit(&queue->shared->writer_idx, memory_order_relaxed);
  if (writer_idx >= reader_idx + queue->shared->element_capacity) {
    // queue full
    return false;
  }
  int64_t idx = writer_idx % queue->shared->element_capacity;
  memcpy(&queue->shared->data[idx * queue->shared->element_size], src_data, queue->shared->element_size);
  atomic_store_explicit(&queue->shared->writer_idx, writer_idx+1, memory_order_release);
  return true;
}

bool spmc_queue_dequeue(spmc_queue_t *queue, uint8_t *dst_data) {
  int64_t reader_idx = atomic_load_explicit(&queue->shared->reader_idx, memory_order_relaxed);
  int64_t writer_idx = atomic_load_explicit(&queue->shared->writer_idx, memory_order_acquire);
  if (reader_idx >= writer_idx) {
    // queue empty
    return false;
  }
  int64_t idx = reader_idx % queue->shared->element_capacity;
  memcpy(dst_data, &queue->shared->data[idx * queue->shared->element_size], queue->shared->element_size);
  atomic_store_explicit(&queue->shared->reader_idx, reader_idx+1, memory_order_release);
  return true;
}