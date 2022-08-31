### How to Write a Reader-Writer Lock?

A correct and high-performant **lock** is essential when we want to go concurrent, multi-threaded. However, the implementation of such a lock is non-trivial.

In this tutorial, we will analyze and draw insights from one version of **Reader-Writer lock** in just 100 lines. The source is credited to Carnegie Mellon University open educational course  [<15445: Database Systems>](https://15445.courses.cs.cmu.edu), which contains such an decent implementation in modern C++. The source file is [here](https://github.com/cmu-db/bustub/blob/master/src/include/common/rwlatch.h). The one in this repo is section-by-section annotated with my comments after analyzing it.

---

#### Requirements

+ There are readers and writers for a data structure.
+ Readers only read from the data structure, while writers will modify the existing data structures.
+ There can only be one writer at a time doing modification.
+ While there is an active writer writing, no reader should be able to access the data structure.
+ When no writer is active, multiple readers could read from the data structure at the same time.

There are versions of the reader-writer locks that **priortize** readers, or writers.

---

#### Language Support

There are a few C++ modern features that would faciliate the implementation of such a lock. We will briefly summarize it below:

1. **std::mutex** is the synchronization primitive. It's not copyable and not movable. User can call `lock` and `unlock` functionality on a share **std::mutex** so that only one thread would be able to grab the lock and proceed. 
2. **std::lock_guard** is the RAII-style wrapper for **std::mutex**. Once created, it will try to grab the ownership for the mutex, and once the lock_guard goes out of scope, it will automatically release the mutex.
3. **std::unique_lock** is a general purpose **std::mutex** wrapper. It's very similar to the **std::lock_guard** above, but more flexible. It could unlock the mutex in the middle of its life cycle, and use with condition variables.
4. **std::condition_variable** is the synchronization primitive that could be many threads to "wait" upon. And operation on this condition variable has the ability to either **notify_one** or **notify_all* of the waiting threads.
5. **Important**: calling `wait` on a condition variable will atomically release the mutex lock it's holding. When it's waken up either by `notify_one()` or `notify_all()`, by that time it will acquire the mutex lock again.

---

#### Source Code

##### declaration

Firstly, we will look at the declaration of such a **Reader-Writer lock**.

The lock relies on one **mute** and two **condition_variable**s for reader and writer respectively. We delete the copy construction and copy assignment operator to make it **non-copyable**. 

```CPP
#pragma once

#include <climits>
#include <condition_variable>
#include <mutex>

class ReaderWriterLatch {
  using mutex_t = std::mutex;
  using cond_t = std::condition_variable;
  static const uint32_t MAX_READERS = UINT_MAX;
public:
  ReaderWriterLatch() = default;
  ~ReaderWriterLatch() { 
    /* grab the lock one more time before destruction
        make sure no more work under progress */
    std::lock_guard<mutex_t> guard(mutex_); 
  }

  /* make lock non-copyable */
  ReaderWriterLatch(const ReaderWriterLatch &) = delete;
  ReaderWriterLatch &operator=(const ReaderWriterLatch&) = delete;

  void WLock();		// acquire the writer lock
  void WUnlock();	// release the writer lock
  void RLock();		// acquire a reader lock
  void RUnlock();	// release a reader lock

private:
  mutex_t mutex_;
  cond_t writer_;
  cond_t reader_;
  uint32_t reader_count_{0};
  bool writer_entered_{false};
```

##### writer lock

```CPP
void ReaderWriterLatch::WLock() {
  std::unique_lock<mutex_t> latch(mutex_);
  while (writer_entered_) {
    /* Only one writer could be writing at the same time */
    reader_.wait(latch);
  }
  /* this thread is the one who are going to write next */
  writer_entered_ = true;
  /* after setting true, no more reader allowed to pass */
  while (reader_count_ > 0) {
    /* let any remaining reader finish their reading first */
    writer_.wait(latch);
  }
}
```

##### writer unlock

notice there are 2 places where threads may be waiting for **reader**:
1. The **WLock()** since only 1 writer could proceed, another writer must wait
2. The **RLock()** since reader cannot read when there is active writer

```CPP
void ReaderWriterLatch::WUnlock() {
  std::lock_guard<mutex_t> guard(mutex_);
  /* Indicate no more writer under progress */
  writer_entered_ = false;
  /* notify that either new writer or new reader could proceed */
  reader_.notify_all();
}
```

##### reader lock

```CPP
void ReaderWriterLatch::RLock() {
  std::unique_lock<mutex_t> latch(mutex_);
  /* 2 conditions to satisfy before move forward */
  while (writer_entered_ || reader_count_ == MAX_READERS) {
    reader_.wait(latch);
  }
  reader_count_++;
}
```

##### reader unlock

```CPP
void ReaderWriterLatch::RUnlock() {
  std::lock_guard<mutex_t> guard(mutex_);
  reader_count_--;
  if (writer_entered_) {
    if (reader_count_ == 0) {
      /* no more reader, one writer can proceed now */
      writer_.notify_one();
    }
  } else {
    if (reader_count_ == MAX_READERS - 1) {
      /* barely recover from fully-loaded */
      reader_.notify_one();
    }
  }
}
```
