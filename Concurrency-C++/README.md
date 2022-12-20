### Concurrency in Modern C++

Ever since the advent of C++11/14 new standards, C++ language finally has its own internal language support for multi-threading. Gone are the days when people have to reply on `pthread` library to write multi-threaded programs.

In this repo, we will go over most details of C++ language supports for concurrency and how to properly apply these in our programs. The main reference book is [\<C++ Concurrency in Action\>](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition) by Anthony Williams. You may find a pdf version of the book in this repo as well.

### Contents

1. [Thread Management](./thread_management.md)
2. [Sharing Data Between Threads](sharing_data_between_threads.md)
3. [Synchronizing Concurrent Operations](synchronizing_concurrent_operations.md)