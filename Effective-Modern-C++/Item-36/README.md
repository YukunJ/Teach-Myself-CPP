### Item 36: Specify **std::launch::async** if asynchronicity is essential

#### Overall
- The default launch policy for **std::async** permits both asynchronous and synchronous task execution.
- This flexibility leads to uncertainty when accessing **thread_local**s, implies that the task may never execute, and affects program logic for timeout-based **wait** calls.
- Specify **std::launch::async** if asynchronous task execution is essential.

There are two standard launch policies applicable to **std::async**:

+ **std::launch::async** means **f** must be run asynchronously, i.e., on a different thread
+ **std::launch::deferred** means **f** may only run when `get` or `wait` is called on the future returned by this call to **std::async**.

It might be surprising to know that the default launch policy for **std::async** are both of the policies mentioned above:

```CPP
// fut1 and fut2 are equivalent expressions
auto fut1 = std::async(f);

auto fut2 = std::async(std::launch::async | std::launch::deferred, f);
```

Using the default launch policy has the following implications:

+ it's not possible to predict whether **f** will run concurrently with **t**.
+ it's not possible to predict whether **f** runs on a thread different from the thread invoking `get` or `wait` on `fut`.
+ it may not be possible to predict whether **f** runs at all.

Such implications mix poorly with the use of **thread_local** variables, and also affect `wait`-based operations on a task because the value yielded will be **std::future\_status::deferred**. A correct version will have to go like this:

```CPP
auto fut = std::async(f);

if (fut.wait_for(0s) == std::future_status::deferred) {
  // if task is deferred
  // use wait or get on fut
  // to call f synchronously
} else {
  while (fut.wait_for(100ms) != std::future_status::ready) {
    // task is running concurrently, will not stuck in infinite loop
  }
}
```

The upshot of these various considerations leads to that, using **std::async** default launch policy is only fine if the followings are satisfied:

+ the task need not run concurrently with the thread calling `get` or `wait`
+ it doesn't matter which thread's **thread_local** variables are read or written
+ Either there's a guarantee that `get` or `wait` will be called on the future returned by **std::async** or it's acceptable that the task may never execute
+ Code using `wait_for` or `wait_until` takes the possibility of deferred status into account

If any of these fails, we might want to guarantee that the task is executed asynchronously by passing **std::launch::async** as the launch policy to **std::async**.

We could write our own template to make a truly asynchronous function call:

```CPP
// C++11
template<typename F, typename... Ts>
inline
std::future<typename std::result_of<F(Ts...)>::type>
reallyAsync(F&& f, Ts&&... params) {
  return std::async(std::launch::async,
                    std::forward<F>(f),
                    std::forward<Ts>(params)...);
}
```