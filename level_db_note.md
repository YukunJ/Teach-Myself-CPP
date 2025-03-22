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
