#include "spsc_queue.hpp"

#include <bit>
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

std::expected<std::unique_ptr<SpscQueue>, SpscError> SpscQueue::create(const char *const path,
                  size_t element_size,
                  size_t element_capacity,
                  SpscMode mode) {

  if (!path ||
      element_size == 0 ||
      element_capacity == 0 ||
      !std::has_single_bit(element_capacity) ||
      (mode != SpscMode::Reader && mode != SpscMode::Writer)) {

    return std::unexpected(SpscError::InvalidArguments);
  }

  // cleanup stale shm from previous writer crash
  if (mode == SpscMode::Writer) {
    shm_unlink(path);
  }

  int oflag = (mode == SpscMode::Reader) ? O_RDWR : O_RDWR | O_CREAT | O_EXCL;

  int raw_fd = shm_open(path, oflag, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

  if (raw_fd == -1) {
    return std::unexpected(SpscError::ShmOpenFailed);
  }

  SpscHeader::Fd fd{raw_fd};

  size_t shared_size = offsetof(SpscShared, data) + element_size * element_capacity;

  if (mode == SpscMode::Writer) {
    if (ftruncate(fd.fd, shared_size) == -1) {
      return std::unexpected(SpscError::FtruncateFailed);
    }
  }

  void* mmap_addr = mmap(nullptr, shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd.fd, 0);

  if (mmap_addr == MAP_FAILED) {
    return std::unexpected(SpscError::MmapFailed);
  }

  SpscHeader::MmappedRegion mmap_region{
      mmap_addr,
      shared_size
  };

  SpscHeader header{
      .fd = std::move(fd),
      .path = std::string{path},
      .mode = mode,
      .mmap_region = std::move(mmap_region),
  };

  auto queue = std::unique_ptr<SpscQueue>(new SpscQueue{std::move(header)});

  if (mode == SpscMode::Writer) {
    queue->shared_.version = kSpscQueueVersion;
    queue->shared_.element_size = element_size;
    queue->shared_.element_capacity = element_capacity;

    queue->shared_.local_writer_idx = 0;
    queue->shared_.local_reader_idx = 0;

    queue->shared_.writer_idx.store(0);
    queue->shared_.reader_idx.store(0);

    queue->shared_.client_connected.store(false);
    queue->shared_.initialized.store(true);
  }

  if (mode == SpscMode::Reader) {

    int attempt = 0;

    while (!queue->shared_.initialized.load()) {

      ++attempt;

      if (attempt == 3) {
        return std::unexpected(SpscError::ConnectionTimeout);
      }

      sleep(10);
    }

    if (queue->shared_.version != kSpscQueueVersion) {
      return std::unexpected(SpscError::VersionMismatch);
    }

    if (queue->shared_.element_capacity != element_capacity) {
      return std::unexpected(SpscError::CapacityMismatch);
    }

    if (queue->shared_.element_size < element_size) {
      return std::unexpected(SpscError::ElementSizeMismatch);
    }

    queue->shared_.client_connected.store(true, std::memory_order_release);
  }

  return queue;
}

SpscQueue::~SpscQueue() noexcept {
  SpscMode mode = header_.mode;
  if (mode == SpscMode::Writer) {
    // writer owns the lifecycle of the queue
    shm_unlink(header_.path.c_str());
  }
  if(mode == SpscMode::Reader) {
    shared_.client_connected.store(false, std::memory_order_release);
  }
}


bool SpscQueue::try_enqueue(const uint8_t *src_data) noexcept{
  if (!shared_.client_connected.load(std::memory_order_acquire)) [[unlikely]] {
    return false;
  }
  assert(mode() == SpscMode::Writer);
  size_t reader_idx = shared_.local_reader_idx;
  size_t writer_idx = shared_.writer_idx.load(std::memory_order_relaxed);

  if (writer_idx >=
      reader_idx + shared_.element_capacity) {
    reader_idx = shared_.reader_idx.load(std::memory_order_acquire);
    shared_.local_reader_idx = reader_idx;
    if (writer_idx >=
        reader_idx + shared_.element_capacity) {
      // queue really full
      return false;
    }

  }

  size_t idx =
      writer_idx & (shared_.element_capacity - 1);

  std::memcpy(&shared_.data[idx * shared_.element_size],
         src_data,
         shared_.element_size);
  shared_.writer_idx.store(writer_idx + 1, std::memory_order_release);
  return true;
}

bool SpscQueue::try_dequeue(uint8_t *dst_data) noexcept {
  assert(mode() == SpscMode::Reader);
  size_t reader_idx = shared_.reader_idx.load(std::memory_order_relaxed);
  size_t writer_idx = shared_.local_writer_idx;

  if (reader_idx >= writer_idx) {
    writer_idx = shared_.writer_idx.load(std::memory_order_acquire);
    shared_.local_writer_idx = writer_idx;
    if (reader_idx >= writer_idx) {
      // queue fully empty
      return false;
    }
  }

  size_t idx =
      reader_idx & (shared_.element_capacity - 1);

  std::memcpy(dst_data,
         &shared_.data[idx * shared_.element_size],
         shared_.element_size);
  shared_.reader_idx.store(reader_idx + 1, std::memory_order_release);
  return true;
}