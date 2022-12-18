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