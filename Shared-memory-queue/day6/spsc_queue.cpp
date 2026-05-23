#include "spsc_queue.hpp"

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// POSIX headers
#include <fcntl.h>
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

spsc_queue_t *spsc_queue_create(const char *const path,
                                size_t element_size,
                                size_t element_capacity,
                                SpscMode mode) {
  if (!path || element_size == 0 || element_capacity == 0 ||
      element_capacity != round_up_power_of_2(element_capacity) ||
      (mode != SpscMode::Reader && mode != SpscMode::Writer)) {

    fprintf(stderr,
            "spsc_queue_create: invalid arguments "
            "(path:%s element_size:%zu element_capacity:%zu mode:%d)\n",
            path, element_size, element_capacity, static_cast<std::underlying_type_t<SpscMode>>(mode));

    return NULL;
  }
  // in case the shared memory queue is still dangling around from last writer crash
  if (mode == SpscMode::Writer) {
      shm_unlink(path); // optional cleanup BEFORE creation
  }
  spsc_queue_t *queue = NULL;
  spsc_shared_t *shared = NULL;

  int fd = -1;

  // use offsetof(spsc_shared_t, data) instead of sizeof(spsc_shared_t)
  // to avoid counting trailing padding of the structure layout
  size_t shared_size =
      offsetof(spsc_shared_t, data) + element_size * element_capacity;

  int oflag =
      (mode == SpscMode::Reader) ? O_RDWR : O_RDWR | O_CREAT | O_EXCL;
  fd = shm_open(path, oflag, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

  if (fd == -1) {
    perror("shm_open");
    goto cleanup;
  }

  if (mode == SpscMode::Writer) {
    int rt = ftruncate(fd, shared_size);

    if (rt == -1) {
      perror("ftruncate");
      goto cleanup;
    }
  }

  queue = static_cast<spsc_queue_t *>(calloc(1, sizeof(spsc_queue_t)));

  if (!queue) {
    perror("calloc");
    goto cleanup;
  }

  shared = static_cast<spsc_shared_t *>(mmap(NULL, shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));

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

  if (mode == SpscMode::Writer) {
    queue->shared->version = kSpscQueueVersion;
    queue->shared->element_size = element_size;
    queue->shared->element_capacity = element_capacity;

    queue->shared->local_writer_idx = 0;
    queue->shared->local_reader_idx = 0;
    queue->shared->writer_idx.store(0);
    queue->shared->reader_idx.store(0);

    queue->shared->client_connected.store(false);
    queue->shared->initialized.store(true);
  }

  if (mode == SpscMode::Reader) {
    int attempt = 0;

    while (!queue->shared->initialized.load()) {
      attempt++;

      if (attempt == 3) {
        fprintf(stderr,
                "spsc_queue: reader mode encountered shared memory not "
                "initialized yet by writer. waited %d attempts. "
                "giving up now\n",
                attempt);

        goto cleanup;
      }

      fprintf(stderr,
              "spsc_queue: reader mode encountered shared memory not "
              "initialized yet by writer. wait 10 seconds...\n");

      sleep(10);
    }

    if (queue->shared->version != kSpscQueueVersion) {
      fprintf(stderr, "spsc_queue: reader expect version %d but see the queue being version %d\n", kSpscQueueVersion, queue->shared->version);
      goto cleanup;
    }

    if (queue->shared->element_capacity != element_capacity) {
      fprintf(stderr,
              "spsc_queue: element_capacity %zu != queue "
              "element_capacity %zu\n",
              queue->shared->element_capacity,
              element_capacity);

      goto cleanup;
    }

    if (queue->shared->element_size < element_size) {
      fprintf(stderr,
              "spsc_queue: element size %zu > queue element size %zu\n",
              element_size,
              queue->shared->element_size);

      goto cleanup;
    }
    queue->shared->client_connected.store(true);
  }

  return queue;

cleanup:
  if (shared && shared != MAP_FAILED) {
    munmap(shared, shared_size);
  }

  if (fd != -1) {
    close(fd);
  }

  if (mode == SpscMode::Writer) {
    shm_unlink(path);
  }

  if (queue) {
    free(queue->header.path);
  }

  free(queue);

  return NULL;
}

void spsc_queue_destroy(spsc_queue_t *queue) {
  int fd = queue->header.fd;
  char *path = queue->header.path;
  SpscMode mode = queue->header.mode;
  if(mode == SpscMode::Reader) {
      queue->shared->client_connected.store(false);
  }
  munmap(queue->shared, queue->header.shared_size);

  close(fd);

  if (mode == SpscMode::Writer) {
    // writer owns the lifecycle of the queue
    shm_unlink(path);
  }
  free(path);
  free(queue);
}

bool spsc_queue_enqueue(spsc_queue_t *queue, uint8_t *src_data) {
  if (!queue->shared->client_connected.load()) {
    return false;
  }

  size_t reader_idx = queue->shared->local_reader_idx;
  size_t writer_idx = std::atomic_load_explicit(&queue->shared->writer_idx, std::memory_order_relaxed);

  if (writer_idx >=
      reader_idx + queue->shared->element_capacity) {
    reader_idx = std::atomic_load_explicit(&queue->shared->reader_idx, std::memory_order_acquire);
    queue->shared->local_reader_idx = reader_idx;
    if (writer_idx >=
        reader_idx + queue->shared->element_capacity) {
      // queue really full
      return false;
    }

  }

  size_t idx =
      writer_idx & (queue->shared->element_capacity - 1);

  memcpy(&queue->shared->data[idx * queue->shared->element_size],
         src_data,
         queue->shared->element_size);
  std::atomic_store_explicit(&queue->shared->writer_idx, writer_idx + 1, std::memory_order_release);
  return true;
}

bool spsc_queue_dequeue(spsc_queue_t *queue, uint8_t *dst_data) {
  size_t reader_idx = std::atomic_load_explicit(&queue->shared->reader_idx, std::memory_order_relaxed);
  size_t writer_idx = queue->shared->local_writer_idx;

  if (reader_idx >= writer_idx) {
    writer_idx = std::atomic_load_explicit(&queue->shared->writer_idx, std::memory_order_acquire);
    queue->shared->local_writer_idx = writer_idx;
    if (reader_idx >= writer_idx) {
      // queue fully empty
      return false;
    }
  }

  size_t idx =
      reader_idx & (queue->shared->element_capacity - 1);

  memcpy(dst_data,
         &queue->shared->data[idx * queue->shared->element_size],
         queue->shared->element_size);
  std::atomic_store_explicit(&queue->shared->reader_idx, reader_idx + 1, std::memory_order_release);
  return true;
}