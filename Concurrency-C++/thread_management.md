### Thread Management


**Overall**
- Starting threads, and various ways of specifying code to run a new thread
- Waiting for a thread to finish versus leaving it to run
- Uniquely identifying threads

--- 

#### Basic thread management 
Every C++ program has at least one thread running, i.e. the main thread running the `main()` function. We can specify to launch new tasks to be executed on new threads.

Starting a thread in modern C++ is as simple as this:

```CPP
void do_some_work();
std::thread my_thread(do_some_work);
```

Alternatively, we can also pass an instance of callable object:

```CPP
class background_task {
public:
  void operator() const {
    do_some_thing();
    do_some_thing_else();
  }
};
background_task f;
std::thread my_thread(f);
```

In this case, the callable object is copied into the storage belonging to the new thread. Therefore, we need to make sure such copy behavor is "side-effect free".

Be ware here that you might encounter the so-called "**C++'s most vexing parse**". For example:

```CPP
// Wrong parse. 
// Actually declare a function
// which takes a pointer to a function that takes no parameter and returns a background_task
// and returns a std::thread
std::thread my_thread(background_task());

// Fix 1: extra brace
std::thread my_thread((background_task()));

// Fix 2: uniform initializer
std::thread my_thread{background_task()};
```

Once a thread is launched, we must decide if we want to wait for its finishing or leave it run on its own. We must decide so before the `std::thread` object is destroyed, otherwise the runtime program will be terminated via `std::thread`'s destructor calls to `std::terminate()`.

If we decide to let the thread running on its own via `detach()` call, we need to make sure the data that the thread is accessing remain valid until the thread finishes.

Let's look at a **BAD** example:

```CPP
struct func {
  int& i_;

  func(int& i): i_(i) {}

  void operator() () {
    for (unsigned j = 0; j < 10; j++) {
      do_something(i_);
    }
  }
};

void oops() {
  int some_local_state = 0;
  func my_func(some_local_state);
  std::thread my_thread(my_func);
  my_thread.detach();
  // thread potentially accesses deallocated local state
}
```

One common approach to handle this scenario is make the thread function **self-contained** and copy any relevant data into the thread instead of sharing the data via pointer or reference.

We can also call `join()` on the thread object to wait for its finishing. The act of calling `join()` cleans up any storage associated with the thread, so the `std::thread` object is no longer associated with the now-finished-and-destroyed thread. This means we can only call `join()` once on a given thread.

As we are obliged to either `detach` or `join` a thread before its destruction, we must make sure on all logical paths of our code, we do make such a decision, even in face of exception. 

A naive approach could be like this:

```CPP
struct func;
void f() {
  int some_local_state = 0;
  func my_func(some_local_state);
  std::thread t(my_func);
  try {
    do_something_in_current_thread();
  } catch (...) {
    t.join();
    throw;
  }
  t.join();
}
```

However, it's not easy and cumbersome to apply `try/catch` on all paths of our code. You might very likely miss one and get your program runtime killed. 

A better approach is resort to the philosophy of "Resource Acquisition Is Initialization(**RAII**)" that wraps up the raw `std::thread` object:

```CPP
class thread_guard {
private:
  std::thread& t_;
public:
  explicit thread_guard(std::thread& t): t_(t) {}
  ~thread_guard() {
    if (t.joinable()) {
      t.join();
    }
  }
  thread_guard(const thread_guard&) = delete;
  thread_guard& operator=(const thread_guard&) = delete;
};

struct func;
void f() {
  int some_local_state = 0;
  func my_func(some_local_state);
  std::thread t(my_func);
  thread_guard g(t); // no more need to explicitely join()
  do_something_in_current_thread();
}
```

Once we detach a thread, its ownership and control are passed over to the C++ Runtime Library which reclaims the resource when the thread exits. 

In order to truly `detach` a thread, there has to be a real undelying thread associated with the `std::thread` you are calling `detach` upon. Therefore, both `join()` and `detach()` call's verification boils down to check the status of function call `joinable()`.

We wraps this subsection by looking at a program that allow editing on multiple files at the same time by spawning a new thread for each new file's edition:

```CPP
void edit_document(const std::string& filename) {
  open_document_and_display_gui(filename);
  while (!done_editing()) {
    user_command cmd = get_user_input();
    if (cmd.type == open_new_document) {
      const std::string new_name = get_filename_from_user();
      std::thread t(edit_document, new_name);
      t.detach(); // achieve autonomy
    } else {
      process_user_input(cmd);
    }
  }
}

```

---

#### Passing arguments to a thread function

Passing arguments to the callable object for thread creation is as simple as supplying additional arguments to the constructor for `std::thread`. But it's important to recognize that by default, the arguments are copied into internal storage for the thread and then passed to the callable object or function as **rvalues** as if they were temporaries.

Here is an example showing this:

```CPP
void f(int i, const std::string& s);

void oops(int some_param) {
  char buffer[1024];
  sprintf(buffer, "%i", some_param);
  std::thread t1(f, 3, buffer); // WRONG
  std::thread t2(f, 3, std::string(buff)); // OK
  t.detach();
}
```

In this case, even if `f` is expecting a second parameter of type `std::string`, it is the `char * buffer` that's passed to the new thread and later on implicitly converted to `std::string`. Therefore, it's very likely that `oops()` function exits before such implicit conversion happens, leading to `buffer` points to invalid memory space.

As we showed in the example, a safer way is to manually do the type conversion before the parameter gets passed into the thread storage space.

A second point is that, it's not easy to get it correct first time when you try to pass in a reference to the new thread. For example, the following code will not be able to compile:

```CPP
// compile error
void update_data_for_widget(widget_id id, widget_data& data);

void oops_again(widget_id w) {
  widget_data data;
  std::thread t(update_data_for_widget, w, data);
  t.join();
}
```

Although the function `update_data_for_widget` is expecting a reference for its second parameter, the `std::thread`'s constructor doesn't know and care about it. The parameter `data` is copied and passed in, and then the internal code tries to pass this rvalue to `update_data_for_widget` and this causes compilation error because you cannot bind a rvalue to non-**const** reference parameter.

The solution is to wrap the arguments in `std:ref` or `std::cref` for const reference-ness:

```CPP
std::thread t(update_data_for_widget, w, std::ref(data));
```

Not only normal functions can be passed to `std::thread`, class member functions are also candidates for this as long as we provide the "`this`" pointer it needs. For example,

```CPP
class X {
public:
  void do_work() {};
};

X my_x;

// this new thread will execute function as if we call
// my_x.do_work();
std::thread t(&X::do_work, &my_x);
```

Some parameters can only be moved but not copied. In this case, we need explicitly to call `std::move` when supplying arguments for the thread constructor. `std::unique_ptr` is a classic example for this scenario.

```CPP
void process_object(std::unique_ptr<big_object>);
std::unique_ptr<big_object> p(new big_object);
p->prepare_data(42);
std::thread t(process_object, std::move(p));
```

By specifying `std::move(p)`, the ownership of `p` is transferred into internal storage of the new thread and then into the function `process_object`.

---

#### Transferring ownership of a thread 

Sometimes we might want to write a function that returns a newly-created thread, or we want to pass a thread handler as an argument to a function. In both cases, we need to transfer the ownership of the thread.

`std::thread` is a move-only class, which means it cannot be copied. Whenever we want to transfer its ownership, we need to explicitly call the move operations on the thread handler.

However, there are some caveats we need to be careful of. Consider the following:

```CPP
void some_function();
std::thread t1(some_function);
std::thread t2(some_function);
// The next line causes runtime termination
// because t1's original thread is neither joined or detached
t1 = std::move(t2);
```

One direct benefits of the move semantics for `std::thread` is that we can build and rely on thread wrapper class and move the thread into the wrapper, so that we are sure their lifecycle (the thread and the wrapper) will be aligned, to avoid any "reference to deallocated space problem".

We can further develop the thread wrapper class we had before:

```CPP
class scoped_thread {
private:
  std::thread t_;
public:
  explicit scoped_thread(std::thread t)
    : t_(std::move(t)) {
    if (!t_.joinable()) {
      throw std::logic_error("No joinable thread");
    }
  }

  ~scoped_thread() { t_.join(); }

  scoped_thread(const scoped_thread&) = delete;
  scoped_thread& operator=(const scoped_thread&) = delete;
};

// usage
struct func; // same as before
void f() {
  int some_local_state = 0;
  scoped_thread t(std::thread(func(some_local_state)));
  do_something_else_in_current_thread();
  // the thread will be joined upon exiting this function frame
}
```

The move support for `std::thread` also allows containers to store thread objects so that we could manage many threads easily:

```CPP
{
  std::vector<std::thread> threads;
  for (unsigned i = 0; i < 20; ++i) {
    threads.emplace_back(do_work, i);
  }

  for (auto& thread: threads) {
    thread.join();
  }
}
```

Actually there is proposal for a `joining_thread` class to be integrated into Standard library, which will join the thread automatically when its destructor is invoked. It has not made it into the STL so far. But we can readily make one by ourselves:

```CPP
class joining_thread {
public:
  std::thread t_;

  joining_thread() noexcept = default;

  template<typename Callable, typename... Args>
  explicit joining_thread(Callable&& func, Args&&... args)
    : t_(std::forward<Callable>(func), std::forward<Args>(args)...) {}

  explicit joining_thread(std::thread t) noexcept: t_(std::move(t)) {}

  joining_thread(joining_thread&& other) noexcept: t_(std::move(other.t_)) {}  

  joining_thread& operator=(joining_thread&& other) noexcept {
    if (joinable()) {
        join();
    }
    t_ = std::move(other.t_);
    return *this;
  }

  joining_thread& operator=(std::thread other) noexcept {
    if (joinable()) {
        join();
    }
    t_ = std::move(other);
    return *this;
  }

  ~joining_thread() noexcept {
    if (joinable()) {
        join();
    }
  }

  void swap(joining_thread& other) noexcept {
    t_.swap(other.t_);
  }

  std::thread::id get_id() const noexcept { return t_.get_id(); }

  bool joinable() const noexcept { return t_.joinable(); }

  void join() { t_.join(); }

  void detach() { t_.detach(); }

  std::thread& as_thread() noexcept { return t_; }

  const std::thread& as_thread() const noexcept { return t_; }
};
```

---

#### Choosing the number of threads at runtime

One very helpful function is `std::thread::hardware_concurrency()`. This functions returns how many hardware threads are supported by our machine. This number is important to know because we don't want to **over-launch** too many threads than what the underlying hardware supports. This is only a hint: it might return `0` if the information is not available.

It's enough to know this single function for this subsection. The following is a code snippet partially achieve what `std::accumulte` does, but does it in parallel. Comments are provided in the code.

```CPP
/* individual accumulate block to be executed on a thread */
template <typename Iterator, typename T>
struct accumulate_block {
  void operator()(Iterator first, Iterator last, T& result) {
    result = std::accumulate(first, last, result);
  }
};

template <typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    const unsigned long length = std::distance(first, last);
    if (!length) {
        // no need to compute
        return init;
    }

    // decide how many threads to launch
    const unsigned long min_per_thread = 25;
    const unsigned long max_threads = (length + min_per_thread -1) / min_per_thread;
    const unsigned long hardware_threads = std::thread::hardware_concurrency();
    const unsigned long thread_nums = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

    // assume divide evenly
    const unsigned long block_size = length / num_threads;
    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads-1);
    Iterator block_start = first;

    // split into small block and assign to threads
    for (unsigned long i = 0; i < (num_threads-1); i++) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread(accumulate_block<Iterator, T>(), block_start, block_end, std::ref(results[i]));
        block_start = block_endl;
    }

    // the main thread also do work
    accumulate_block<Iterator, T>()(block_start, last, results[num_threads-1]);

    // harvest all threads and combine the results
    for (auto& t: threads) {
        t.join();
    }
    return std::accumulate(results.begin(), results.end(), init);
}
```

---

#### Identifying threads

Each thread has its own identifier of type `std::thread::id` and could be retrieved in two ways:

1. Call the function `get_id()` on a `std::thread` object
2. Call function `std::this_thread::get_id()` to get the id of the current running thread

This class `std::thread::id` could be freely copied and compared. The STL provides it with a total ordering and it could also be used as key in associative container like `std::map`, `std::set`. The STL also provides `std::hash<std::thread::id>`, so it can also act as key in `std::unordered_map` and `std::unordered_set`.

Sometimes, based on whether we are on the master thread, code could do different things:

```CPP
std::thread::id master_thread;

void some_core_part_of_algorithm() {
  if (std::this_thread::get_id() == master_thread) {
    do_master_thread_work();
  } else {
    do_common_work();
  }
}
```
