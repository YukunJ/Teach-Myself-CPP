## leveldb note

#### Slice

`Slice` is the basic unit of reference to storage. It contains a pointer to a continugous array of bytes and its size, without owning it. In some senses it's similiar to `std::string_view`. 

The important point to remember is that, when using a `Slice`, the user should ensure the underlying external storage `data_` is valid. and concurrent usage of non-const methods on the same `Slice` should be externally synchronized.