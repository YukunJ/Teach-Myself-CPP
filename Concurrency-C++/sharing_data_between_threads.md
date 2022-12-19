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
