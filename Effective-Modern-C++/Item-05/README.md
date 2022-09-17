### Item 05: Prefer **auto** to explicit type declarations

#### Overall
- **auto** variables must be initialized, are geneally immune to type mismatches that can lead to portability or efficiency problems, can ease the process of refactoring, and typically require less typing than variables with explicitly specified types.
- **auto**-typed variables are subject to the pitfalls described in Item 2 and 6.

Since C++, **auto** keyword helps to deduce variable type from their initializer, which implies that you must explicitly initialize a variable. This would be a good practice to avoid potential usage of uninitialized variable.

```CPP
int x1;		// potentially uninitialized

auto x2;	// error! initializer required

auto x3 = 0;	// fine, x's value is well-defined

```

And it saves a lot of trouble typing. Consider the following two pieces of codes that do the same thing:

```CPP
// Version1: old style
template<typename It>
void dwim(It b, It e) {
  while (b != e) {
    typename std::iterator_traits<It>::value_type currValue = *b;
    ...
  }
}

// Version2: C++11 auto-enabled
template<typename It>
void dwim2(It b, It e) {
  while (b != e) {
    auto currValue = *b;
    ...
  }
}
```

It's more than obvious that version 2 is both easy to type and easy to read. Double win!

What's more, because **auto** uses type deduction, it can represent types known only to compilers:

```CPP
// C++11
auto derefUpLess = 
  [](const std::unique_ptr<Widget>& p1,
     const std::unique_ptr<Widget>& p2)
  { return *p1 < *p2; }

// C++14 parameter of lambda may also be auto
auto derefLess = 
  [](const auto& p1, const auto& p2)
  { return *p1 < *p2; }
```

Lambda function defines a **closure** in C++. An **auto**-declared variable holding a closure has the same type as the closure, and as such it uses only as much memory as the closure requires.

Some people prefer to use **std::function** which is a template in C++11 that generalizes the idea of a function pointer. It can not only contain function pointer, but any callable object. However, using **std::function** is not only a pain to type, but it might also takes more memory to store the closure, which takes a fixed size for any signature.

```CPP
// tedious and may take more memory
std::function<bool(const std::unique_ptr<Widget>&, 
                   const std::unique_ptr<Widget>&>
    derefUpLess = [](const std::unique_ptr<Widget>& p1,
                     const std::unique_ptr<Widget>& p2)
                    { return *p1 < *p2; }
```

**auto** not only saves typing, but also helps to avoid "type shortcuts" problems. For example, it's common to write such codes:

```CPP
std::vector<int> v;
...
unsigned sz = v.size();
```

the official return type of `v.size()` is **std::vector<int>::size_type**. **unsigned** aligns with it on 32-bit machines. However, on 64-bit machine, **unsigned** is 32-bit while **std::vector<int>::size_type** is 64-bit. This may lead to bad portability and hard bugs to fix.


We can look at another classic example:

```CPP
std::unordered_map<std::string, int> m;
...
for (const std::pair<std::string, int>& p : m) {
  ...
}
```

There is something inefficient with the code. Be honest it's hard to spot at first glance. The key of a **std::unordered_map** is *const*, so the type of **std::pair** in the hash table is actually **std::pair<const std::string, int>**. As a result of inconsistency, the compiler will copy each object in `m` into a temporary object, and then bind the reference `p` to that temporary object, which will be destroyed by the end of loop iteration.

**auto** come to save us again:

```CPP
for (const auto& p : m) {
  ....
}
```

This is not only easy to type, but also make sure that we do have the "binding" to the real element in the map, but just a temporary copy.

Of course, **auto** has its own drawbacks, which will be further explored and discussed in Item02 and Item06.