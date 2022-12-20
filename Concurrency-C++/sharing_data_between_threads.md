### Sharing Data Between Threads

**Overall**
- Problems with sharing data between threads
- Protecting data with mutexes
- Alternative facilities for protecting shared data

--- 

#### Problems with sharing data between threads

Blindly sharing data between different threads can cause us a lot of problems. All these boil down to modifying the data. Mere reading data is unaffected by whether or not another thread is reading the same data.

One important concept programmers often use is **invariant**, a property that is always true about some data strucutures or program running status. These invariants are often temporarily broken during a data modification, causing problems in face of concurrent execution threads. 

For example, consider how to delete a node from a doubly-linked list. Suppose we have four nodes **A <-> B <-> C <-> D** and we want to remove **B**. It breaks into three steps:

1. change **A.next** points to **C**
2. change **C.prev** points to **A**
3. release **B**

One invariant in a doubly-linked list is that, a node's next's prev should be this node itself, and a node's prev's next should be this node itself. However, when we are done with step 1 but not step 2, clearly this invariant is broken. **A**'s next points to **C**, but **C**'s prev is still **B**. If there is another thread executing logic on this doubly-linked list and relies on this invariant, problem arises.

There is another concept called **race condtion**. In programming world, **race condition** is defined as something where the outcome depends on the relative ordering of execution of operations on two or more threads. Some race conditions are benign, while others lead to undefined behavior.

Typicaly, there are two approaches to deal with race conditions:
+ apply protection mechanism to ensure only the thread performing modification could see the temporary broken invariant. (this chapter)
+ switch to use so-called ***lock-free*** data structure so that modifications are done as a series of indivisible changes, each of which preserves the invariants. (later chapter)

---

#### Protecting shared data with mutexes

The basic idea of protecting shared data is to create **mutual exclusion**, i.e. only one thread can be doing critical work that might temporarily break invariants. If another thread want to join, they have to wait until the first guy finishes (thus restore invariants).

In C++, we achieve so by creating an instance of **std::mutex** and call `lock()` and `unlock()` functions on it. However, it is not recommended to use the raw **std::mutex** because you have to remember manually `lock()` and `unlock()`. Instead, we prefer **RAII** style wrapper for mutex that unlocks the mutex upon destruction. **std::lock_guard** and C++17 **std::scoped_lock** are such wrappers we need.

```CPP
#include <list>
#include <mutex>
#include <algorithm>

std::list<int> some_list;
std::mutex some_mutex;

void add_to_list(int new_value) {
  std::lock_guard<std::mutex> guard(some_mutex);
  some_list.push_back(new_value);
}

void list_contains(int value_to_find) {
  std::lock_guard<std::mutex> guard(some_mutex);
  return std::find(some_list.begin(), some_list.end(), value_to_find) != some_list.end(); 
}
```

Notice in real-world practice, it's more common to group the data and mutex into a single class rather than using global variables like the example above. 

Also we need to be super careful about if we leak pointers or references of the shared data structure to the outside world. *Any code that has access to that pointer or reference can now access (and potentially modify) the protected data without locking the mutex*.

Moreover, there are other forms of race conditions that cannot be easily protected by a **mutex**. Consider a **std::stack<int>**. Suppose there is an operation we want to do on it:

```CPP
std::stack<int> stack;

int operation() {
  int value = NOT_EXIST;
  if (!stack.empty()) {
    value = stack.top();
    stack.pop();
  }
  return value;
}
```

If the stack currently only has 1 element, and two threads execute `operation()` and pass the cardinality check `!stack.empty()`, one of them will get the last element and the other will try to pop from an empty stack that causes problems. In a sense that `empty()` is not reliable in face of concurrency.

Plus, in case where the value type of the **std::stack<T>** is big, memory allocation might throw exception for constructing the return value from the `operation()` function. The `top` might already get `pop()`ed out but fails to be returned back from the function. We essentially lose this piece of data permanently.

A few options exist with their own pros and cons:

1. Pass in a reference to be populated. This requires to construct an instance of the stack's value type prior to the call.
2. Require a no-throw copy constructor or move constructor. Not very likely to get most user-defined classes to be no-throw in copy constructor or move constructor.
3. Return a smart pointer to the popped item. We have to pay the price for memory allocation and its life cycle management.

Generally speaking, we should allow more flexibility in the design. The following is an example implementation of a thread-safe stack wrapper class that provides option 1 and 3 at the same time.

```CPP
#include <exception>
#include <memory>
#include <mutex>
#include <stack>

struct empty_stack: std:exception {
  const char* what() const throw();
};

template<typename T>
class threadsafe_stack {
public:
    threadsafe_stack() {}

    threadsafe_stack(const threadsafe_stack& other) {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }

    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }

    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        const std::shared_ptr<T> res(std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }

    void pop(T& value) {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        value = data.top();
        data.pop();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }

private:
    std::stack<T> data;
    mutable std::mutex m;
};
```

Sometimes we might want to reduce lock contention by providing a fine-grained locking scheme. However, it might lead to deadlock problems. Let's take a lock.

Without getting too academic, a deadlock situation is where two or more threads holding some locks that the other wants, and they are not preemptive (not willing to give up the lock voluntarily). Consider there are two locks **A** and **B**. An operation takes both locks to ensure invariant. Thread 1 takes lock **A** first, then thread 2 takes lock **B**. Now they're stuck waiting for the other side to give up the locks.

A general discipline for acquiring multiple locks is that, we should acquire them in a pre-defined order.

Thanks to modern C++ new STL support, **std::lock** allows us to lock two or more **std::mutex** all at once: either all locked or none is locked. Consider the following code sample when we need to swap a pair of objects:

```CPP
class some_big_object;
void swap(some_big_object& lhs, some_big_object& rhs);

class X {
private:
  some_big_object some_detail_;
  std::mutex m_;
public:
  X(const some_big_object& sd): some_detail_(sd) {}
  friend void swap(X& lhs, X& rhs) {
    if (&lhs == &rhs) {
      return;
    }
    std::lock(lhs.m_, rhs.m_);
    std::lock_guard<std::mutex> lock_a(lhs.m_, std::adopt_lock);
    std::lock_guard<std::mutex> lock_b(rhs.m_, std::adopt_lock);
    swap(lhs.some_detail_, rhs.some_detail_);
  }
};
```

Furthermore, **C++17** has provided a more powerful **RAII** style template **std:scoped_lock** that can lock multiple **mutex**es in constructor atomically. So we could simplify the code to:

```CPP
// C++11
...
    std::lock(lhs.m, rhs.m);
    std::lock_guard<std::mutex> lock_a(lhs.m_, std::adopt_lock);
    std::lock_guard<std::mutex> lock_b(rhs.m_, std::adopt_lock);
...

-----------

// C++17
...
    std::scoped_lock<std::mutex, std::mutex> guard(lhs.m_, rhs.m_);
...
```

Notice although the **std::lock** and **std::scoped_lock<>** can help avoid deadlock where we want to acquire two or more **mutex**es together, it doesn't help if they're acquired separately in different functions. In that case, software developers have to rely on discipline as follows:

+ Avoid nested locks. Try to strcuture design such that each thread only needs to take at most one mutex at a time. Try not to take another while you already have one.
+ Avoid calling user-supplied code while holding a lock.
+ Acquire locks in a fixed order.
+ Use a lock hierarchy.

There is another type of lock wrapper in C++ that offers more flexibility -- **std::unique_lock**. It doesn't always own the mutex that it's associated with. Instead, we can pass **std::adopt_lock** as the second argument to indicate it's already locked, or **std::defer_lock** to tell constructor do not try to acquire the lock upon construction. We could later on call **lock()** member function on the **std::unique_lock** instance. And we can also explicitly **unlock()** the mutex before the destruction of the **std::unique_lock** for better concurrency performance when appropriate. The **std::unique_lock** instance could be passed to **std::lock** function as well. So our previous two-mutex example could also be written using **std::unique_lock**:

```CPP
// using std::unique_lock
...
    std::unique_lock<std::mutex> lock_a(lhs.m, std::defer_lock);
    std::unique_lock<std::mutex> lock_b(rhs.m, std::defer_lock);
    std::lock(lock_a, lock_b);
...
```

The size of a **std::unique_lock** is typically larger than **std::lock_guard** because it has to store the flag of whether the associated mutex has been acquired or not by calling **owns_lock()** function, and needs to do sanity check on every operation. Therefore, there is a slight performance penalty when using **std::unique_lock** over **std::lock_guard**.

**std::unique_lock** is a move-only class that allows the transfer of ownership by move semantics, since it does not have to own its associated mutex. One typical use is to allow a function to lock a mutex and transfer the ownership of that lock back to the caller:

```CPP
std::unique_lock<std::mutex> get_lock() {
  extern std::mutex some_mutex;
  std::unique_lock<std::mutex> lk(some_mutex);
  prepare_data();
  return lk;
}

void process_data() {
  std::unique_lock<std::mutex> lk(get_lock());
  do_something();
}
```

---

So far we have talked about how to use **std::mutex** and various wrapper classes around it to safely sharing data between threads. There are also some rare cases worth discussion, in which, if just merely adopting **mutex** for protection, could suffer unnecessary performance penalty.

First scenario is during initialization. Some data structures are read-only. The only possible race condition happen during its initializtion if multiple threads try to initialize it. Consider the following scenario:

```CPP
// dummy version
std::shared_ptr<some_resource> resource_ptr;
std::mutex resource_mutex;
void foo() {
  std::unique_lock<std::mutex> lk(resource_mutex);
  if (!resource_ptr) {
    resource_ptr.reset(new some_resource);
  }
  resource_ptr->do_something();
}
```

The lock mechanism is only needed if the resource has not been initialized. After that, the locking is unnecessary. But the above code pays for the locking cost anyway.

The C++ Standard Library provides **std::once_flag** and **std::call_once** to help in this scenario, where we can pass an Callable object to **std::call_once** for initialization and we could be assured that it will be only called once among different threads.

```CPP
std::shared_ptr<some_resource> resource_ptr;
std::once_flag resource_flag;

void init_resource() {
  resource_ptr.reset(new some_resource);
}

void foo() {
  std::call_once(resource_flag, init_resource()); // called exactly once
  resource_ptr->do_something();
}
```

The second scerario is where there are a lot of concurrent read but only a small amound of write. We wish to allow multiple readers to process concurrently, but read and write operations must be mutually exclusive. This actually reduces to the famous reader-writer locking mechanism. Since C++17, Standard Library has support for this scenario using **std::shared_mutex** and **std::shared_lock<>**.

Let's see an example adopting the reader-writer locking mechanism for the DNS Cache lookup and update:

```CPP
#include <map>
#include <string>
#include <mutex>
#include <shared_mutex>

class dns_entry;
class dns_cache {
private:
  std::map<std::string, dns_entry> entries;
  mutable std::shared_mutex entry_mutex;
public:
  dns_entry find_entry(const std::string& domain) const {
    std::shared_lock<std::shared_mutex> lk(entry_mutex); // shared mode
    const std::map<std::string, dns_entry>::const_iterator it = entries.find(domain);
    return (it == entries.end()) ? dns_entry(); it->second;
  }

  void update_or_add_entry(const std::string& domain, const dns_entry& dns_details) {
    std::lock_guard<std::shared_mutex> lk(entry_mutex); // exclusive mode
    entries[domain] = dns_details;
  }
};
```