### Item 37: Make **std::thread**s unjoinable on all paths

#### Overall
- Make **std::threads** unjoinable on all paths.
- **join**-on-destruction can lead to difficult-to-debug performance anomalies.
- **detach**-on-destruction can lead to difficult-to-debug undefined behavior.
- Declare **std::thread** objects last in listsof data memebers.

There are four cases that make a **std::thread** unjoinable:

+ **Default-constructed std::threads**
+ **std::thread objects that have been moved from**
+ **std::thread objects that have been joined**
+ **std::thread objects that have been detached**

The standard requires that, if the destructor for a joinable thread is invoked, the whole execution of the program is terminated, because unjoined thread could lead to very tricky situation that people should strive to avoid.

Given the above information, suppose we have a function to do some filtering first and then do some computations on the filtered result if a certain condition is met.

```CPP
constexpr auto tenMillion = 10'000'000; // C++14

// return whether computation is performed
bool doWork(std::function<bool(int)> filter, int maxVal = tenMillion) {

  std::vector<int> goodVals;

  std::thread t([&filter, maxVal, &goodVals]
                {
                 for (auto i = 0; i <= maxVal; ++i)
                   { if (filter(i)) goodVals.push_back(i); }
                 });
  ...
  auto nh = t.native_handle();
  ...
  if (conditionAreSatisfied()) {
    t.join();
    performComputation(goodVals);
    return true;
  }

  // oops! We forget to join the thread, whole program terminated
  return false;
}
```

As commented above, we forget to join the thread on the `false` branch, which will lead to our program be terminated if we hit that branch.

Generally speaking, it's very hard to manually check and make sure on every branch the thread is joined as `return`, `continue`, `break`, `goto` or exception will make it complicated. The approach we should take is to wrap the thread into an object following the **RAII** principle.

```CPP
class ThreadRAII {
public:
  enum class DtorAction { join, detach };

  // initialize the thread last, in case it gets executed right away
  ThreadRAII(std::thread&& t, DtorAction a)
   : action(a), t(std::move(t)) {}

  ~ThreadRAII() {
    if. (t.joinable()) {
      if (action == DtorAction::join) {
        t.join();
      } else {
        t.detach();
      }
    }
  }
  
  // support moving
  ThreadRAII(ThreadRAII&&) = default;
  ThreadRAII& operator=(ThreadRAII&&) = default;

  std::thread& get() { return t; }

private:
  DtorAction action;
  std::thread t;
};
```

Now we can replace the raw **std::thread** in our "join-leaking" code snippet above with our **ThreadRAII** class happily and not worry about forgetting to join the thread on every path of our code logic.

