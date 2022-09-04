### Item 17: Understand special member function generation


#### Overall
- The special member functions are those compilers may generate on their own: default constructor, destructor, copy operations and move operations.
- Move operations are generated only for classes lacking explicitly declared move operations, copy operations, and a destructor.
- The copy constructor is generated only for class lacking an explicitly declared copy constructor, and it's deleted if a move operation is declared. The copy assignment operator is generated only for classes lacking an explicitly declared copy assignment operator, and it's deleted if a move operation is declared. Generation of the copy operations in classes with an explicitly declared destructor is deprecated.
- Member function templates never suppress generation of special member functions.

Dating back to C++98, many of us are quite familiar with the "Big 3": copy constructor, copy assignment operator and destructor. Plus the constructor. If you don't specifically provide implementations of these 4 memebr functions of a class, the compiler will generate for you.

Things are a bit different and more complex since C++11, with the introduction of move semantics. There are two more member functions: move constructor and move assignment operator. The default generated ones by compiler will try to do "member-wise move". However, this is best-attempt move, since not necessarily all the class members are "move-able". If not, the copy operations are applied instead.

The two **copy** operations are independent. If you declare one, compiler will still generate the other for one. But the two **move** operations are not independent, your declaration of any will prevent the compiler generating the other.

Moreover, your declaration of **copy** operations and **destructor** also prevent the compiler generating two **move** operations. This goes in the other way as well: if you declare **move** operations by yourself, compiler won't generate **copy** operations for you anymore.

A good practice is to explicitly indicate that you want the compiler to generate the default member functions for you by using `= default` keyword. For example:

```CPP
class Base {
public:
  virtual ~Base() = default;	// make dtor virtual

  Base(Base&&) = default;	// support moving
  Base &operator=(Base&&) = default;

  Base(const Base&) = default;	// support copying
  Base &operator=(Base&&) = default;
  
  ...
};
```

Although it requires you to type more words, but it could save a lot of bugs in the future. Consider the case that you have a data structure that permits fast lookup:

```CPP
class StringTable {
public:
  StringTable() {}
  ...

private:
  std::map<int, std::string> values;
};
```

One day you think we need add some logs when constructing and destructing the `StrtingTable`, so you revise the class as follows:

```CPP
class StringTable {
public:
  StringTable() { Log("Create a String Table"); }
  ~StringTable() { Log("Delete a String Table"); }
  ...

private:
  std::map<int, std::string> values;
}
```

This seemingly harmless changes actually bring about problem. In the original version, the compiler would generate move operations for us. However, in the new version, since we explicitly declare the destructor, no more move operations generated for us.

This means that any `std::move` call on this data structure would ultimately invoke copy operations on the map, which could be orders of magnitude slower than moving it.

One last thing is that, all the rules we mention doesn't apply to "member function template". It won't prevent compiler from generating special member functions for us, even though these templates could be instantiated to produce the signature for the copy constructor, move constructor, etc.

The below summarizes the rules in C++11:
+ Default constructor: Same rules as C++98. Generated only if the class contains no user-declared constructors.
+ Destructor: Essentially same rules as C++98; sole difference is that destructors are noexcept by default. As in C++98, virtual only if a base class destructor is virtual.
+ Copy constructor: Same runtime behavior as C++98: memberwise copy construction of non-static data members. Generated only if the class lacks a user-declared copy constructor. Deleted if the class declares a move operation. Generation of this function in a class with a user-declared copy assignment operator or destructor is deprecated.
+ Copy assignment operator: Same runtime behavior as C++98: memberwise copy assignment of non-static data members. Generated only if the class lacks a user-declared copy assignment operator. Deleted if the class declares a move operation. Generation of this function in a class with a user-declared copy constructor or destructor is deprecated.
+ Move constructor and move assignment operator: Each performs memberwise moving of non-static data members. Generated only if the class contains no user-declared copy operations, move operations, or destructor.
