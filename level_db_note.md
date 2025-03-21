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
