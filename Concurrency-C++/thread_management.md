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