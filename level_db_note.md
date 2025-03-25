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