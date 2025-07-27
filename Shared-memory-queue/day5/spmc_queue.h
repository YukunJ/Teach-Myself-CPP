#ifndef SPMC_QUEUE_H
#define SPMC_QUEUE_H

#include <stdalign.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define SPMC_QUEUE_VERSION 0
#define L1_DCACHE_LINESIZE 64 // found via /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size

enum spmc_mode { spmc_mode_reader = 0, spmc_mode_writer };

typedef struct spmc_header {
  int fd;
  char *path;
  enum spmc_mode mode;
  size_t shared_size;
} spmc_header_t;

// the writer_idx and reader_idx do not handle arithmetic overflow
// hence the queue should not deal with more than 2^64-1 elements before reset
typedef struct spmc_shared {
  uint8_t version;
  size_t element_capacity;
  size_t element_size;
  atomic_bool initialized;
  atomic_bool client_connected;
  alignas(L1_DCACHE_LINESIZE) atomic_size_t writer_idx;
  alignas(L1_DCACHE_LINESIZE) atomic_size_t reader_idx;
  alignas(L1_DCACHE_LINESIZE) uint8_t data[];
} spmc_shared_t;

typedef struct spmc_queue {
  spmc_header_t header;
  spmc_shared_t *shared;
} spmc_queue_t;

spmc_queue_t *spmc_queue_create(const char *const path, size_t element_size, size_t element_capacity,
                                enum spmc_mode mode);
void spmc_queue_destroy(spmc_queue_t *queue);
bool spmc_queue_enqueue(spmc_queue_t *queue, uint8_t *src_data);
bool spmc_queue_dequeue(spmc_queue_t *queue, uint8_t *dst_data);

#endif // SPMC_QUEUE_H