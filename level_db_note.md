## leveldb note

#### Slice

`Slice` is the basic unit of reference to storage. It contains a pointer to a continugous array of bytes and its size, without owning it. In some senses it's similiar to `std::string_view`. 

The important point to remember is that, when using a `Slice`, the user should ensure the underlying external storage `data_` is valid. and concurrent usage of non-const methods on the same `Slice` should be externally synchronized.

#### BloomFilterPolicy

`BloomFilterPolicy` is the leveldb built-in bloom filter, which implements the interface `FilterPolicy`. There are 2 functionalities:

+ `void CreateFilter(const Slice* keys, int n, std::string* dst)`

This is basically the encoding part about how to encode the `n` keys into the `dst`. It will calculate a probe factor `k = ln2 * bits_per_key` and place that at the end of the destination place for later `KeyMayMatch` (decoding) part to retrieve.

For each key, it will encode the whole destination `k` times using the formula

```cpp
h = BloomHash(key);
delta = (h >> 17) | (h << 15); // Rorate right 17 bits
for (i = 0; i < k; i++) {
    bit_position = h % bits;
    destination[bit_position / 8] |= (1 << (bit_position % 8));
    h += delta;
}
```

+ `bool KeyMayMatch(const Slice& key, const Slice& bloom_filter)`

This is the "decoding" part. We can first retrieve back the probe factor `k = bloom_filter.data[len-1]` and reversely check if each bit_position is as encoded as it does in the `CreateFilter` stage.

```cpp
h = BloomHash(key);
delta = (h >> 17) | (h << 15); // Rorate right 17 bits
for (i = 0; i < k; i++) {
    bit_position = h % bits;
    expect bloom_filter.data[bit_postion / 8] as the bit (1 << (bit_position % 8)) set
    h += delta;
}
```

#### Coding Schema

`coding.h` and `coding.cc` describe the Endian-neutral encoding mechanism used in leveldb. It has fixed-length mode and variable length mode.

For `fixed-length` mode, it ensures to encode the least-significant byte first for `int32` and `int64`. For Slice, it encodes the size of the slice first and then byte by byte encoding. A test case vividly demonstates this:

```cpp
std::string dst;
PutFixed32(&dst, 0x04030201);
ASSERT_EQ(4, dst.size());
ASSERT_EQ(0x01, static_cast<int>(dst[0]));
ASSERT_EQ(0x02, static_cast<int>(dst[1]));
ASSERT_EQ(0x03, static_cast<int>(dst[2]));
ASSERT_EQ(0x04, static_cast<int>(dst[3]));
```

For `variable-length` mode, it use 7 bits in each byte to encode the real data, and the most significant bit (8th bit) in the byte to indicate if more data is upcoming in the next byte. The benefits of this encoding mode is that it saves a lot of space for small value encoding in a large type. The worst case is that it would use 5 bytes for `int32` and 10 bytes for `int64`. 

Let's take a very simple example, we want to encode a int64 of value `0b 10101010`. We take a first 7 least-significant bits `0101010` and set up the 8th bit since we have 1 more bit unencoded. and the remaining bit `1` we will encoding it in the next byte, witht the 8th bit of that byte unset to indicate the end of encoding. So eventually we only use 2 bytes to encode this `int64`: `(first byte) 1010 1010 (second byte) 0000 0001`

#### Arena

`Arena` is the internal memory pool for leveldb. It's non-copyable. It keeps tracking of blocks of size `4096` or more, and releases them upon destruction. 

The main APIs are `char* Allocate(size_t bytes)` and `char* AllocateAligned(size_t bytes)`, the latter of which provides memory allocated with same alignment as `void *` (8 bytes typically). 

For `Allocate`, when the requesting bytes are smaller than the remaining bytes in the current block `alloc_ptr_`, it will return `alloc_ptr_` and increment its position by the requesting bytes. If the requesting bytes are more than the remaining bytes, it breaks down into 2 cases:

1. When requesting more than `4096/4=1024` bytes, Arena will directly allocate exact requesting byte block and return to caller.
2. Otherwise, Arena will give up the remaining space in the current block, allocate a new block of `4096` and allocates the requesting bytes from this new block back to the caller, and keep track of this new block in `alloc_ptr_`.

For `AllocateAligned` it is essentially the same operation, plus little adjustment to ensure the returning address is properly aligned with `align = (sizeof(void*) > 8) ? sizeof(void*) : 8;`

#### Cache

leveldb provides a builtin LRUCache implementing the `Cache.h` interface. 

##### HandleTable

leveldb folks handcrafted a simple hashtable storing the `LRUHandle *`. In essence, it is a list of buckets, each bucket is a chaining linked list.

`LRUHandle** list_` points to a list `length_` of buckets chaining the `LURHandle*`. It aims to keep a low average linked list length <= 1 by ensuring there are as many bucket as number of entries in the hashtable. 

For accessing, it breaks into 2 steps. (1) Find the bucket of chain. (2) Iterate through the chain. We can look an example:

```cpp
LRUHandle** FindPointer(const Slice& key, uint32_t hash) {
  LRUHandle** ptr = &list_[hash & (length_ - 1)];
  while (*ptr != nullptr && ((*ptr)->hash != hash || key != (*ptr)->key())) {
    ptr = &(*ptr)->next_hash;
  }
  return ptr;
}
```

The `hash & (length_ -1)` finds which bucket it is going to. And then it follows the hash chain to iterate through, either find the target `key` or reach the end.

The fact that `FindPointer` gives back `LRUHandle **` is handy. Take the `Remove` function for example:

```cpp
LRUHandle* Remove(const Slice& key, uint32_t hash) {
  LRUHandle** ptr = FindPointer(key, hash);
  LRUHandle* result = *ptr;
  if (result != nullptr) {
    *ptr = result->next_hash;
    --elems_;
  }
  return result;
}
```

We know there might be some other `LRUHandle *` whose `->next_hash` points to `*ptr`. By modifying in-place `*ptr = result->next_hash`, whoever points to the to-be-removed LRUHandle as their `next_hash` will have the correct `result->next_hash` value. Think of this as a left-shift by 1 position in an array.

##### LRUCache

This is the single shard base class of the final built-in LURCache that leveldb folks provide. The public key APIs are `Lookup`, `Release`, `Erase` and `Prune`.

It internally keeps 2 doubly-linked list `lru_` and `in_use_`, which are mutually exclusive. What's in `in_use_` means some clients currently hold reference to that `Cache::Handle*`, and the entries in that list `in_use_` does not have a particular order. What's in `lru_` is sorted by least-recent-used priority that is not currently used by client. The minimal reference count to keep is 1 in `lru_` and `>1` to be in `in_use_`.

The logic here is, when client `Lookup` and get a `Cache::Handle*`, it will be moved to the `in_use_` list if not already there. When they are done with that cache entry and call `Release`, the reference count will drop by 1. When it reaches `1`, which means there is no client referencing this entries except for the Cache object, it will be moved from `in_use_` back to the `lru_`'s head, indicating it is the most recently accessed cache entry. After client insert a new cache entry, it will perform cache evicting base on the LRU policy.

##### ShardedLRUCache

This is the final version of the leveldb built-in LRU cache. Basically it maintains an arry of 16 `LRUCache` we discussed above in an effort to further balance-split the traffic.  Upon an `Insert` or `Lookup`, it will first hash the key and bitwise `&` operation the last 4 bits to pick the corresponding single `LRUCache` instance and pass along the operations.

#### Comparator

`Comparator` interface is used to compare and give ordering between 2 `Slice`s in leveldb. There is one built-in `BytewiseComparator` that already implements this interface. 

There are 2 interesting APIs from the `Comparator` interface. `FindShortestSeparator(std::string* start, const Slice& limit)` which aims to change `start` to a middle value sitting in between `[start, limit)` if `start` is smaller than `limit`. `FindShortSuccessor(std::string *key)` changes `key` to be a short string that's greater than or equal to `*key`.

For the `BytewiseComparator` it will find the common prefix and increment the next byte by 1 to find the shortest separator (assume `*start > limit`) and increment the first byte that could be incremented (the byte not equal to `0xff`).

Notice the usage of singleton pattern and `NoDestructor` trait in initializing the `Bytewise Comparator`. We will talk about that next.

```cpp
const Comparator* BytewiseComparator() {
  static NoDestructor<BytewiseComparatorImpl> singleton;
  return singleton.get();
}
```

#### NoDestructor

`NoDestructor` is a template class leveldb defines to wrap an instance whose destructor would never be called. It leverages several advanced C++ features, including

+ use perfect forwarding for variadic parameter passing to the instance's constructor
+ in-place `new` operator to create the instance into this `NoDestructor`'s private variable space `instance_storage_t`
+ guarantee this space is of the same alignment as the to-be-created instance type and is large enough to hold the instance

A good question to ask is: OK. So why these wrapped instances should not be destructed? One key point is, **the destruction order of static local variables across different compilaiton units are not guaranteed in C++**. If there are dependencies between different static variables and the destruction order is incorrect, it may lead to undefined behavior.

#### Histogram

`Histogram` is the quick histogram class that leveldb implements for measurement and benchmark purpose. Usually you cannot afford to record every single data point to be able to compute the percentile. An approximation is to bucket it.

Here `Histogram` has `154` buckets ranging from `1` to `1e200`. When inserting a number into the histogram, it find the first bucket `k` such that `kBucketLimit[k] > value` and then do a `bucket[k]++`. Basically it rounds this `value` to be `kBucketLimit[k]`.

When querying a percentile, it simply finds 2 buckets that sandwich the target percentile, and do a linear interpolation in between.

#### Status

`Status` encapsulates the result of an operation in leveldb. It could either be OK, or a type of error with a customizable message. The possible codes include `kOK`, `kNotFound`, `kCorruption`, `kNotSupported`, `kInvalidArgument` and `kIOError`. 

The design to support customized message while being resource-efficient is via its `status_` char array. When the status is `kOk`, the `status_` is `nullptr`. Otherwise, it is of the format that `status[0..3]` 4 bytes to represent the message length. `status_[4]` is the error code. and `status_[5..]` is the customized error message.

#### Posix Env

The `env_posix.cc` implements the `env.h` interface that provides a bunch of file operations and `FileLock`, `Logger`, `RandomAccessFile`, `SequentialFile` and `WriteableFile` abstractions in POSIX environment.

The `Limiter` limits the resouce usage on readonly and mmap file. It is thread-safe.

#### SkipList

`skiplist.h` implements one of the core data structures in leveldb -- SkipList. For a detailed illustration of what is a skiplist, we can refer to my old article on [How to design a Key-Value store using **SkipList**](https://github.com/YukunJ/Teach-Myself-CPP/blob/main/KV-store-skiplist/README.md) or another awesome blog [https://selfboot.cn/en/2024/09/09/leveldb_source_skiplist/#Principles-of-Skip-Lists)

One striking feature is the variable-length array `next_` within a fixed struct definition `Node`, leveraging manual memory allocation to accommodate the actual required size. The `next_` variable keeps the next node's link at different levels, with `next_[0]` being the lowest level link. It's to handle the problem that `max_length` might increase as the skiplist grows and therefore it's hard to define a static fixed size of this `next_` array.


```cpp
// declaration
 private:
  // Array of length equal to the node height.  next_[0] is lowest level link.
  std::atomic<Node*> next_[1];

// allocation
template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode(
    const Key& key, int height) {
  char* const node_memory = arena_->AllocateAligned(
      sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
  return new (node_memory) Node(key);
}
```

### LogWriter & LogReader

In the `log_writer.cc` it implements the writer class that emits each operation record to a backup physical file. It has a few interesting points:

+ Each block written is at most **32768** bytes long, so a record does not necessarily fit into a block
+ Therefore each record has a corresponding type: `kFullType`, `kFirstType`, `kLastType` and `kMiddleType`. As the name indicates, `kFullType` means this record fully fit in this block. `kFirstType` means more to come. `kLastType` means this is the last piece for this record. `kMiddleType` means it is in the middle of the current writing record.
+ Each written record has a 7 byte header occupied by 4 bytes of CRC checksum and 2 bytes of record length and 1 byte of the record type.

The `log_reader.cc` basically does the inverse of what the writer does. It reads back the bytes from the log file from disk and reassembly them into a record. It takes into considerations that, based on the record type, a record might break into multiple block of log files on disk.