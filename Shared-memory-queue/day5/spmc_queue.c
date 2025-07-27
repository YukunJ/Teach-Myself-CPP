#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "spmc_queue.h"
#include <assert.h>
#include <errno.h> // assume POSIX: errno is set on alllocation failure
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

static size_t round_up_power_of_2(size_t n) {
  size_t pow = 1;
  while (pow < n) {
    pow *= 2;
  }
  return pow;
}

spmc_queue_t *spmc_queue_create(const char *const path, size_t element_size, size_t element_capacity,
                                enum spmc_mode mode) {
  if (!path || element_size == 0 || element_capacity == 0 || element_capacity != round_up_power_of_2(element_capacity) ||(mode != spmc_mode_reader && mode != spmc_mode_writer)) {
    fprintf(stderr, "spmc_queue_create: invalid arguments (path:%s element_size:%zu element_capacity:%zu mode:%d\n",
            path, element_size, element_capacity, mode);
    return NULL;
  }
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
    perror("calloc");
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
    memset(queue->shared, 0, queue->header.shared_size);
    queue->shared->version = SPMC_QUEUE_VERSION;
    queue->shared->element_size = element_size;
    queue->shared->element_capacity = element_capacity;
    atomic_store(&queue->shared->writer_idx, 0);
    atomic_store(&queue->shared->client_connected, false);
    atomic_store(&queue->shared->initialized, true);
  }
  if (mode == spmc_mode_reader) {
    int attempt = 0;
    while (!atomic_load(&queue->shared->initialized)) {
      attempt++;
      if (attempt == 3) {
        fprintf(stderr,
                "spmc_queue: reader mode encounters shared memory not initialized yet by writer. waited %d attempts. "
                "giving up now\n",
                attempt);
        goto cleanup;
      }
      fprintf(stderr,
              "spmc_queue: reader mode encounters shared memory not initialized yet by writer. wait 10 seconds...\n");
      sleep(10);
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
    atomic_store(&queue->shared->reader_idx, 0);
    atomic_store(&queue->shared->client_connected, true);
  }
  return queue;

cleanup:
  if (fd != -1)
    close(fd);
  munmap(shared, shared_size);
  if (mode == spmc_mode_writer)
    shm_unlink(path);
  if (queue)
    free(queue->header.path);
  free(queue);
  return NULL;
}

void spmc_queue_destroy(spmc_queue_t *queue) {
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
}

bool spmc_queue_enqueue(spmc_queue_t *queue, uint8_t *src_data) {
  if (!queue->shared->client_connected) {
    return false;
  }
  size_t reader_idx = atomic_load_explicit(&queue->shared->reader_idx, memory_order_acquire);
  size_t writer_idx = atomic_load_explicit(&queue->shared->writer_idx, memory_order_relaxed);
  if (writer_idx >= reader_idx + queue->shared->element_capacity) {
    // queue full
    return false;
  }
  size_t idx = writer_idx & (queue->shared->element_capacity - 1);
  memcpy(&queue->shared->data[idx * queue->shared->element_size], src_data, queue->shared->element_size);
  atomic_store_explicit(&queue->shared->writer_idx, writer_idx + 1, memory_order_release);
  return true;
}

bool spmc_queue_dequeue(spmc_queue_t *queue, uint8_t *dst_data) {
  size_t reader_idx = atomic_load_explicit(&queue->shared->reader_idx, memory_order_relaxed);
  size_t writer_idx = atomic_load_explicit(&queue->shared->writer_idx, memory_order_acquire);
  if (reader_idx >= writer_idx) {
    // queue empty
    return false;
  }
  size_t idx = reader_idx & (queue->shared->element_capacity - 1);
  memcpy(dst_data, &queue->shared->data[idx * queue->shared->element_size], queue->shared->element_size);
  atomic_store_explicit(&queue->shared->reader_idx, reader_idx + 1, memory_order_release);
  return true;
}