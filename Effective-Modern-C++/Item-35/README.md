### Item 35: Prefer task-based programming to thread-based

#### Overall
- The **std::thread** API offers no direct way to get return values from asynchronously run functions, and if those functions throw, the program is terminated.
- Thread-based programming calls for mannual management of thread exhaustion, oversubscription, load balancing, and adaptation to new platforms.
- Task-based programming via **std::async** with the default launch policy handles most of these issues for you.

If you want to run a function `doAsyncWork` asynchronously, you have basically two options: either creating a **std::thread** for that or employing the **std::async** to achieve that:

```CPP
int doAsyncWork();

// std::thread approach
std::thread t(doAsyncWork);

// std::async approach
auto fut = std::async(doAsyncWork);
auto result = fut.get(); // fetch the result once it's ready
```

We claim that generally task-based approaching using **std::async** is preferred over thread-based approach using **std::thread**.

Task-based approach frees you from thread management. Software threads are a limited resources. You may get an exception for creating a thread because there is no more available one. And the thread might get terminated if your function throws exception.

Moreover, heavy context switching caused by **over-subscription** (more ready-to-run software threads than hardware threads) causes high cache miss rate and degrades program performance. You might get a better management by delegating such missions to the library implementors.

Moreover, using **std::async**, if using the default launch policy, your task might be executed in a new thread, or the current one if there is no more available threads or the current thread is under-utilized compared to others. If you really want it to be launched in a new thread, you could specify **std::launch::async** launch policy.

In short, compared to thread-based programming, a task-based design spares you the travails of manual thread management, and it provides a natural way to examine the results of asynchronously executed functions (i.e. return values or exception). 

Nevertheless, there are some rare cases where you might want to manage the thread yourself:

+ **You need access to the API of the underlying threading implementation**

+ **You need to and are able to optimize thread usage for your application**

+ **You need to implement threading technology beyond the C++ concurrency API**

