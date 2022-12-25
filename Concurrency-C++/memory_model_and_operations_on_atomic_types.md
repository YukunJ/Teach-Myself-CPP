### Memory Model and Operations on Atomic Types


**Overall**
- The details of the C++ memory model
- The atomic types provided by the C++
- Standard Library
- The operations that are available on those types
- How those operations can be used to provide synchronization between threads

--- 

#### Memory model basics

There are four important things to notice about the C++ basic objects and memory locations:

+ Every variable is an object, including those that are members of other objects
+ Every oject occupies at least one memory location
+ Variables of fundamental types such at int or chat occupy exactly one memory location, whatever their size, even if they're adjacent or part of an array
+ Adjacent bit fields are part of the same memory location

Problems arise in concurrent program when two or more threads are accessing **the same memory location** simultaneously without any protection, and one of the access is write access (All read operations would be fine).

In order to avoid the race condition, there has to be an enforced ordering between accesses. We can either do this by mutual exclusion using a **mutex** or atomic operations to be described in this chapter later on.


#### Atomic operations and types in C++

Not all **std::atomic\<T\>** types are that performant. Some have internal locks to guarantee the atomicity. So in that cases, it's no different from using a **std::mutex** to protect the critical session. We can query the property by the member function **is_lock_free()**. Since C++17, all atomic types have a **static constexpr** member variable **X::is_always_lock_free**.

##### std::atomic_flag

This is the simplest atomic type that guarantee always lock-free. It is so simple that supports only a few operations: **clear()**, **test_and_set()**. We can implement a spinlock using it:

```CPP
class spinlock_mutex {
public:
  spinlock_mutex(): flag(ATOMIC_FLAG_INIT) {}
  void lock() {
    while (flag.test_and_set(std::memory_order_acquire));
  }
  void unlock() {
    flag.clear(std::memory_order_release);
  }
private:
  std::atomic_flag flag;
};
```

##### std::atomic\<bool\>

Next simplest boolean type of atomics. It might not be lock-free and use an internal lock depending on STL implementation.

##### std::atomic\<T*\>

Aside from normal atomic opetations, it also supports **fetch_add()** and **fetch_sub()**, which mimics the pointer arthimetics.

##### atomic integral types

It supports a more comprehensive set of operations, including **+=**, **-=**, **&=**, **|=**, **^=**, **++x**, **x++**, **--x**, **x--**, etc.

##### std::atomic\<\> primary class template

User can create an atomic variant of a user-defined types. For example, if there is a class **Widget**, then there could be an atomic variant **std::atomic\<Widget\>**. But it has to fulfill quite a few requirements:

+ not have any virtual functions or virtual base classes
+ use compiler-generated copy-assignment operator
+ every base class and non-**static** data member must have trival copy-assignment operator

But the more complex the class gets, the more likely that we should just use **std::mutex** for data protection.

#### Synchronizing operations and enforcing ordering

**Skipped**. Too detailed and complicated for now. Saved for future.