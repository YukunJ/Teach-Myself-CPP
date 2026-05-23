#ifndef SPSC_QUEUE_H
#define SPSC_QUEUE_H

#include <atomic>
#include <cstddef>
#include <cstdint>

#ifdef __cpp_lib_hardware_interference_size
  constexpr std::size_t kCacheLineSize = std::hardware_destructive_interference_size;
#else
  // Fallback for older compilers or environments where it's not defined
  constexpr std::size_t kCacheLineSize = 64;
#endif

constexpr uint8_t kSpscQueueVersion = 0;

enum class SpscMode { Reader, Writer };

typedef struct spsc_header {
  int fd;
  char *path;
  SpscMode mode;
  size_t shared_size;
} spsc_header_t;

// the writer_idx and reader_idx do not handle arithmetic overflow
// hence the queue should not deal with more than 2^64-1 elements before reset
typedef struct spsc_shared {
  uint8_t version;
  size_t element_capacity;
  size_t element_size;
  std::atomic<bool> initialized;
  std::atomic<bool> client_connected;

  // local idx is to reduce the read frequency of the shared idx to save cache coherence traffic
  alignas(kCacheLineSize) size_t local_writer_idx;
  alignas(kCacheLineSize) size_t local_reader_idx;
  
  alignas(kCacheLineSize) std::atomic<size_t> writer_idx;
  alignas(kCacheLineSize) std::atomic<size_t> reader_idx;

  alignas(kCacheLineSize) uint8_t data[];
} spsc_shared_t;

typedef struct spsc_queue {
  spsc_header_t header;
  spsc_shared_t *shared;
} spsc_queue_t;

spsc_queue_t *spsc_queue_create(const char *const path,
                                size_t element_size,
                                size_t element_capacity,
                                SpscMode mode);

void spsc_queue_destroy(spsc_queue_t *queue);

bool spsc_queue_enqueue(spsc_queue_t *queue, uint8_t *src_data);

bool spsc_queue_dequeue(spsc_queue_t *queue, uint8_t *dst_data);

#endif // SPSC_QUEUE_H