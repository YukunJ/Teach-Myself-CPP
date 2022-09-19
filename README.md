# Teach-Myself-CPP

Hey! Welcome to this repository. This is the notes-keeping place for teaching myself C++, modern C++ in specific. It might also include some auxiliary topics about useful tools.

I learned traditional C++98 a few years ago and didn't quite work on this programming language recently. In order to better prepare myself for workplace once I graduate from school, I will teach myself modern C++ in leisure time and update notes here. Hopefully, this notes could also help other people wishing to get onboard with modern C++.

Aside from book reading notes, mostly I will be borrowing good resources from all over the repos on github, and re-implement them for deepening understanding. Sources are pointed out in each on.

## Content

- [Git tutorial](./Git-tutorial)
- [Makefile tutorial](./Makefile-tutorial)
- [How to intergate with **GoogleTest** using CMake](./CMake-googletest-tutorial)
- [How to write a Reader-Writer lock](./ReaderWriter-lock)
- [How to design a Key-Value store using SkipList](./KV-store-skiplist)
- [How to write Smart pointers](./Smart_ptr)
    * [my **unique_ptr**](./Smart_ptr/README.md#unique_ptr)
    * [my **shared_ptr**](./Smart_ptr/README.md#shared_ptr)
- [Effective Modern C++](./Effective-Modern-C++/Effective-Modern-CPP.pdf)
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
        * [Use **constexpr** whenever possible](./Effective-Modern-C++/Item-15)
        * [Understand special member function generation](./Effective-Modern-C++/Item-17)
	* Smart Pointers
		* [Use **std::unique_ptr** for exclusive-ownership resource management](./Effective-Modern-C++/Item-18) 
        * [Use **std::shared_ptr** for shared-ownership resource management](./Effective-Modern-C++/Item-19)       
	* Rvalue References, Move Semantics, and Perfect Forwarding
        * [Understand **std::move** and **std::forward**](./Effective-Modern-C++/Item-23)
        * [Distinguish universal references from rvalue references](./Effective-Modern-C++/Item-24)
        * [Use **std::move** on rvalue references, **std::forward** on universal references](./Effective-Modern-C++/Item-25)
	* Lambda Expressions
	* The Concurrency API
	* Tweaks
