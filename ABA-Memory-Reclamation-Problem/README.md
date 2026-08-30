# ABA-Memory-Reclamation-Problem

This small project demonstrates the **ABA problem** and **memory reclamation problem** in lock-free programming, using a lock-free stack as an example.

At a high level, two common approaches are used to address these problems:

1. **Versioning**

   * **Tagged Pointer** — prevents the ABA problem, but does **not** solve memory reclamation.

2. **Deferred Memory Reclamation**

   * **Hazard Pointers** — solves both ABA and memory reclamation.
   * **Epoch-Based Reclamation (EBR)** — solves both ABA and memory reclamation.

## Files

### `lockfree_stack_bug.cpp`

A seemingly-correct lock-free stack implementation that has both:

* an **ABA problem**
* a **memory reclamation problem**

The ABA problem occurs because the stack's head is represented only by a pointer. A node can be removed and its memory address later reused, causing a CAS to incorrectly believe that the head has not changed.

The memory reclamation problem occurs because a thread may still hold a pointer to a node after another thread removes and deletes it. The first thread can then dereference freed memory.

### `lockfree_stack_tagged_pointer.cpp`

A lock-free stack using a **tagged pointer**:

```cpp
struct TaggedPointer {
    Node* node_;
    uint64_t tag_;
};
```

The pointer and version tag are updated together using a 128-bit CAS. Every successful update increments the tag:

```text
A, tag=0
   ↓
B, tag=1
   ↓
A, tag=2
```

Therefore, even if the pointer changes from `A → B → A`, the complete value is different:

```text
(A, 0) != (A, 2)
```

so a CAS can detect that the head has changed.

On x86-64 systems supporting `CMPXCHG16B`, the 128-bit tagged pointer can be updated atomically using the hardware `lock cmpxchg16b` instruction. Check for `cx16` in the CPU flags:

```bash
lscpu
```

For example:

```text
Flags: ... cx16 ...
```

In this environment, GCC routes the 16-byte atomic operation through `libatomic`, which ultimately contains `CMPXCHG16B`:

```bash
objdump -d /lib/x86_64-linux-gnu/libatomic.so.1 | grep cmpxchg16b
```

Example:

```text
4205: f0 48 0f c7 0f    lock cmpxchg16b (%rdi)
```

**Important:** the tagged pointer prevents ABA, but it does **not** solve memory reclamation. A popped node still cannot necessarily be `delete`d immediately because another thread may still hold a reference to it.

The next step is therefore to combine the lock-free data structure with a memory reclamation strategy such as **Hazard Pointers** or **Epoch-Based Reclamation**.
