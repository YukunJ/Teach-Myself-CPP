#ifndef SPSC_QUEUE_H
#define SPSC_QUEUE_H
#include <atomic>
#include <new>
#include <string>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <sys/mman.h>

#ifdef __cpp_lib_hardware_interference_size
  constexpr std::size_t kCacheLineSize = std::hardware_destructive_interference_size;
#else
  // Fallback for older compilers or environments where it's not defined
  // assume 64 bytes of L1-D cache line size
  constexpr std::size_t kCacheLineSize = 64;
#endif

static_assert(std::atomic<bool>::is_always_lock_free);
static_assert(std::atomic<size_t>::is_always_lock_free);

constexpr uint8_t kSpscQueueVersion = 0;

enum class SpscMode { Reader, Writer };

// ---------------------------
// Process-local metadata only
// --------------------------
struct SpscHeader {
  // RAII wrapper for file descriptor
  struct Fd {
    int fd = -1;
    explicit Fd(int fd = -1) : fd{fd} {};
    ~Fd() {
      if (fd != -1) {
        close(fd);
      }
    }
    // no copy, move only
    Fd(const Fd &) = delete;
    Fd &operator=(const Fd &) = delete;
    Fd(Fd &&other) : fd{other.fd} { other.fd = -1; }
    Fd &operator=(Fd &&other) {
      if (this != &other) {
        if (fd != -1) { close(fd); }
        fd = other.fd;
        other.fd = -1;
      }
      return *this;
    }
  };

  // RAII wrapper for mmap region
  struct MmappedRegion {
    void *addr = MAP_FAILED;
    size_t mapped_size = 0;
    MmappedRegion(void *addr = MAP_FAILED, size_t size = 0) : addr{addr}, mapped_size{size} {}
    ~MmappedRegion() { if (addr != MAP_FAILED) { munmap(addr, mapped_size); } }
    // no copy, move only
    MmappedRegion(const MmappedRegion &) = delete;
    MmappedRegion &operator=(const MmappedRegion &) = delete;
    MmappedRegion(MmappedRegion &&other) : addr{other.addr}, mapped_size{other.mapped_size} {
      other.addr = MAP_FAILED;
      other.mapped_size = 0;
    }
    MmappedRegion &operator=(MmappedRegion &&other) {
      if (this != &other) {
        if (addr != MAP_FAILED) { munmap(addr, mapped_size); }
        addr = other.addr;
        mapped_size = other.mapped_size;
        other.addr = MAP_FAILED;
        other.mapped_size = 0;
      }
      return *this;
    }
  };
  Fd fd;
  std::string path;
  SpscMode mode;
  MmappedRegion mmap_region;
};

// ---------------------------
// Shared memory layout (MUST be POD)
// ---------------------------
struct SpscShared {
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
};
static_assert(std::is_trivially_copyable_v<SpscShared>);
static_assert(std::is_standard_layout_v<SpscShared>);

class SpscQueue {
public:
  SpscHeader header;
  SpscShared *shared;
};

SpscQueue *spsc_queue_create(const char *const path,
                                size_t element_size,
                                size_t element_capacity,
                                SpscMode mode);

void spsc_queue_destroy(SpscQueue *queue);

bool spsc_queue_enqueue(SpscQueue *queue, uint8_t *src_data);

bool spsc_queue_dequeue(SpscQueue *queue, uint8_t *dst_data);

#endif // SPSC_QUEUE_H