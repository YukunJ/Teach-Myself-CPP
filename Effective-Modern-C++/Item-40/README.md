### Item 40: Use **std::atomic** for concurrency, **volatile** for special memory

#### Overall
- **std::atomic** is for data accessed from multiple threads without using mutexes. It's a tool for writing concurrent software.
- **volatile** is for memory where reads and writes should not be optimized away. It's a tool for working with special memory.

People have confusions about the usage between **std::atomic** and **volatile**.

**std::atomic**'s operations appear as if they were protected under a **mutex** critical session. 

Firstly, they guarantee atomic operations:

```CPP
std::atomic<int> ai(0);

ai = 10;		// atomically set to 10

std::cout << ai;	// atomically read ai's value

++ai;			// atomically increment ai to 11
```

Secondly, they put restrictions on the order of executions. What does this mean? Consider the following code where we use a shared bool flag to indicate if a task has completed and signal another concurrent thread:

```CPP
std::atomic<bool> valAvailable(false);

auto imptValue = computeImportantValue();

valAvailable = true; // concurrent thread could start now
```

Although we know it's important that we finish `computeImportantValue()` before setting the `valAvailable = true`, compiler doesn't know anything about this. It might switch the order of these two lines's execution, for potentially better performance.

Luckily we use **std::atomic** here. One restriction is that, no code that, in the source code, precedes a write of a **std::atomic** variable may take place afterwards. This is actually ***sequential consistency***.

On the other, then, what does **volatile** do? It's for dealing with special memory that doesn't behave normally.

Under normal situation, compiler might optimize the following code:

```CPP
int x;

// only assign once
auto y = x;
y = x;

// only assign 20
x = 10;
x = 20;
```

However, such optimization is not appropriate when we are dealing with special memory, for example ***memory-mapped I/O***. Consider if `x` corresponds to a control port for a radio transmitter or temperature sensor. 

By declaring the variable **volatile**, we tell compiler and underlying hardware to not optimize these operations for us and retain them in origin form.

One more point, since **std::atomic** and **volatile** serve different purposes, they can even be used together:

```CPP
volatile std::atomic<int> vai;
```

This could be useful if `vai` corresponds to a memory-mapped I/O location that was concurrently accessed by multiple threads.