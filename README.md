# Teach-Myself-CPP

Hey! Welcome to this repository. This is the notes-keeping place for teaching myself C++, modern C++ in specific. It might also include some auxiliary topics about useful tools.

I learned traditional C++98 a few years ago and didn't quite work on this programming language recently. In order to better prepare myself for workplace once I graduate from school, I will teach myself modern C++ in leisure time and update notes here. Hopefully, this notes could also help other people wishing to get onboard with modern C++.

Aside from book reading notes, mostly I will be borrowing good resources from all over the repos on github, and re-implement them for deepening understanding. Sources are pointed out in each one.

I finished reading on 

+ [\<Effective C++\>](https://www.oreilly.com/library/view/effective-c-55/0321334876/)

+ [\<More Effective C++\>](https://www.oreilly.com/library/view/more-effective-c/9780321545190/)

+ [\<Effetive STL\>](https://www.oreilly.com/library/view/effective-stl/9780321545183/)

+ [\<Effective Modern C++\>](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/)

+ [\<C++ Concurrency in Action\>](https://www.manning.com/books/c-plus-plus-concurrency-in-action)

+ [\<Design Data-Intensive Application\>](https://www.oreilly.com/library/view/designing-data-intensive-applications/9781491903063/)

Currently I am reading the comprehensive book [\<C++ Software Design: Design Principles and Patterns for High-Quality Software\>](https://www.oreilly.com/library/view/c-software-design/9781098113155/) to learn some design principles and best practices.

## Content

- [Google C++ Style Guidance](https://google.github.io/styleguide/cppguide.html)
- [Git tutorial](./Git-tutorial)
- [Gdb tutorial](./Gdb-tutorial)
- [Makefile tutorial](./Makefile-tutorial)
- [Socket Programming tutorial](./Socket-Programming)
- [How to intergate with **GoogleTest** using CMake](./CMake-googletest-tutorial)
- [How to write a Reader-Writer lock](./ReaderWriter-lock)
- [How to design a Key-Value store using **SkipList**](./KV-store-skiplist)
- [How to design a multi-user chatroom using **Socket Programming**](https://github.com/YukunJ/TurtleChat)
- [How to write Smart Pointers](./Smart_ptr)
    * [my **unique_ptr**](./Smart_ptr/README.md#unique_ptr)
    * [my **shared_ptr**](./Smart_ptr/README.md#shared_ptr)
- C++ New Features Clarification
	* [Variadic Template](./Modern-C++-features/varadic_template)
- [**<C++ Concurrency in Action>**](./Concurrency-C++/C++_Concurrency_in_Action.pdf)
	* [Thread Management](./Concurrency-C++/thread_management.md)
	* [Sharing Data Between Threads](./Concurrency-C++/sharing_data_between_threads.md)
    * [Synchronizing Concurrent Operations](./Concurrency-C++/synchronizing_concurrent_operations.md)
	* [Memory Model and Operations on Atmoic Types](./Concurrency-C++/memory_model_and_operations_on_atomic_types.md)
- [**<Effective Modern C++>**](./Effective-Modern-C++/Effective-Modern-CPP.pdf)
	* Deducing Types
        * [Understand template type deduction](./Effective-Modern-C++/Item-01)
        * [Understand ***auto*** type deduction](./Effective-Modern-C++/Item-02)
		* [Understand ***decltype***](./Effective-Modern-C++/Item-03)
		* [Know how to view deduced types](./Effective-Modern-C++/Item-04)
	* **auto**
		* [Prefer **auto** to explicit type declarations](./Effective-Modern-C++/Item-05)
		* [Use the explicitly typed initializer idiom when **auto** deduces undesired types](./Effective-Modern-C++/Item-06)
	* Moving to Modern C++
		* [Distinguish between **()** and **{}** when creating objects](./Effective-Modern-C++/Item-07)
        * [Prefer ***nullptr*** to **0** and ***NULL***](./Effective-Modern-C++/Item-08)
		* [Prefer alias declarations to ***typedef***s](./Effective-Modern-C++/Item-09)
		* [Prefer scoped **enum**s to unscoped **enum**s](./Effective-Modern-C++/Item-10)
		* [Prefer **delete**d functions to private undefined ones](./Effective-Modern-C++/Item-11)
		* [Declare overriding functions **override**](./Effective-Modern-C++/Item-12)
		* [Prefer **const_iterator**s to **iterator**s](./Effective-Modern-C++/Item-13)
		* [Declare functions **noexcept** if they won't emit exceptions](./Effective-Modern-C++/Item-14)
        * [Use **constexpr** whenever possible](./Effective-Modern-C++/Item-15)
		* [Make **const** member functions thread safe](./Effective-Modern-C++/Item-16)
        * [Understand special member function generation](./Effective-Modern-C++/Item-17)
	* Smart Pointers
		* [Use **std::unique_ptr** for exclusive-ownership resource management](./Effective-Modern-C++/Item-18) 
        * [Use **std::shared_ptr** for shared-ownership resource management](./Effective-Modern-C++/Item-19)
		* [Use **std::weak_ptr** for **std::shared_ptr**-like pointers that can dangle](./Effective-Modern-C++/Item-20)   
		* [Prefer **std::make_unique** and **std::make_shared** to direct use of **new**](./Effective-Modern-C++/Item-21)   
		* [When using the **Pimpl** Idiom, define special member functions in the implementation file](./Effective-Modern-C++/Item-22)   
	* Rvalue References, Move Semantics, and Perfect Forwarding
        * [Understand **std::move** and **std::forward**](./Effective-Modern-C++/Item-23)
        * [Distinguish universal references from rvalue references](./Effective-Modern-C++/Item-24)
        * [Use **std::move** on rvalue references, **std::forward** on universal references](./Effective-Modern-C++/Item-25)
		* [Avoid overloading on universal references](./Effective-Modern-C++/Item-26)
		* [Familiarize yourself with alternatives to overloading on universal references](./Effective-Modern-C++/Item-27)
		* [Understand reference collapsing](./Effective-Modern-C++/Item-28)
		* [Assume that move operations are not present, not cheap, and not used](./Effective-Modern-C++/Item-29)
		* [Familiarize yourself with perfect forwarding failure cases](./Effective-Modern-C++/Item-30)
	* Lambda Expressions
		* [Avoid default capture modes](./Effective-Modern-C++/Item-31)
		* [Use init capture to move objects into closures](./Effective-Modern-C++/Item-32)
		* [Use **decltype** on **auto&&** parameters to **std::forward** them](./Effective-Modern-C++/Item-33)
		* [Prefer lambdas to **std::bind**](./Effective-Modern-C++/Item-34)
	* The Concurrency API
		* [Prefer task-based programming to thread-based](./Effective-Modern-C++/Item-35)
		* [Specify **std::launch::async** if asynchronicity is essential](./Effective-Modern-C++/Item-36)
		* [Make **std::thread**s unjoinable on all paths](./Effective-Modern-C++/Item-37)
		* [Be aware of varying thread handle destructor behavior](./Effective-Modern-C++/Item-38)
		* [Consider **void** futures for one-shot event communication](./Effective-Modern-C++/Item-39)
		* [Use **std::atomic** for concurrency, **volatile** for special memory](./Effective-Modern-C++/Item-40)
- [**<C++ Software Design Principles>**](https://www.oreilly.com/library/view/c-software-design/9781098113155/)
	* [Understand the Importance of Software Design](./C++-software-design/item1.md)
	* [Design for Change](./C++-software-design/item2.md)
	* [Separate Interfaces to Avoid Artificial Coupling](./C++-software-design/item3.md)
	* [Design for Testability](./C++-software-design/item4.md)
	* [Design for Extension](./C++-software-design/item5.md)
	* [Adhere to the Expected Behavior of Abstractions](./C++-software-design/item6.md)
	* [Understand the Similarities Between Base Classes and Concepts](./C++-software-design/item7.md)
	* [Understand the Semantic Requirements of Overload Sets](./C++-software-design/item8.md)
	* [Pay Attention to the Ownership of Abstractions](./C++-software-design/item9.md)
	* [Consider Creating an Architectural Document](./C++-software-design/item10.md)
	* [Understand the Purpose of Design Patterns](./C++-software-design/item11.md)
	* [Beware of Design Pattern Misconceptions](./C++-software-design/item12.md)
	* [Design Patterns Are Everywhere](./C++-software-design/item13.md)