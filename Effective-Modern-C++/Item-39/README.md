### Item 39: Consider **void** futures for one-shot event communication

#### Overall
- For simple event communication, condvar-based designs requires a superfluous mutex, impose constraints on the relative progress of detecting and reacting tasks, and requires reacting tasks to verify that the event has taken place.
- Designs employing a flag avoid those problems, but are based on polling, not blocking.
- A condvar and flag can be used together, but the resulting communications mechanism is somewhat stilted.
- Using **std::promise** and future dodges these issues, but the approach uses heap memory for shared states, and it's limited to one-shot communication.

Sometimes we need to send a signal to a second, concurrent running thread some information. For example, a computation has completed so that the asynchronous thread could start doing something else. 

A polling approach with a **std::atomic** variable would be too CPU costy. Another traditional approach uses a conditional variable along with a mutex. And it's easy to make it wrong because

+ If the detecting task **notifies** the condvar before the reacting task **wait**s, the reacting task will hang.
+ The **wait** statement fails to account for spurious wakeups.

The correct implementation of the traditional way is:

```CPP
std::condition_variable cv;
std::mutex m;

bool flag(false);
...
// the detecting side
{
  std::lock_guard<std::mutex> g(m);
  flag = true;
}
cv.notify_one();

...
// the reacting side
...
{
  std::unique_lock<std::mutex> lk(m);
  // use lambda to avoid supurious wakeups
  cv.wait(lk, [] { return flag; });
}

```

However, with modern C++, for a one-shot communication, we could just discard all these mutex and condvar, and reply only on a **std::promise**.

```CPP
std::promise<void> p;

// the detecting side
{
  p.set_value();
}

// the reacting side
{
  p.get_future().wait();
}
```

Like the condvar-based approach, the reacting task is truly blocked after making the **wait** call, so it consumes no system resources.

The only constraint on this approach is that, it's for **one-shot** communication. It cannot be used repeatedly.