### ABA-Memory-Reclamation-Problem

In this small project, we will demonstrate what is ABA problem and memory reclamation problem in lock-free programming, using the implementation of a lock-free stack as an example.

On a high level, over the past decades people have come up with 2 general methods for tackling these problems: 

1. Version Control
    1. Tagged Pointer (only solve ABA problem)
2. Deferred Reclamation
    1. Hazard Pointer (solve both)
    2. Epoch-based Reclaimation (solve both)

Files:

1. lockfree_stack_bug.cpp: a seemingly OK implementation that has both ABA and Memory Reclamation problems