### Item 11: Prefer **delete**d functions to private undefined ones


#### Overall
- Prefer deleted functions to private undefined ones.
- Any function may be deleted, including non-member functions and template instantiations.

C++ would define some "special member functions" for you if you don't do so in a class. However, sometimes you want to disable the usage of such function. 

The old C++98-style way achieves this by declaring this function as **private** and not implement it. For example, in C++98 the **basic_ios** is implemented as follows:

```CPP
template <class charT, class traits = char_traits<charT> > 
class basic_ios : public ios_base {
public:
  ...
private:
  basic_ios(const basic_ios&);			// not defined
  basic_ios& operator=(const basic_ios&);	// not defined
}
```

Since C++11, the usage of **delelte** keyword should be preferred over the old style:

```CPP
template <class charT, class traits = char_traits<charT> > 
class basic_ios : public ios_base {
public:
  ...
  basic_ios(const basic_ios&) = delete;
  basic_ios& operator=(const basic_ios&) = delete;
}
```

By convention, **delete**d functions should be declared in **public**, not **private**. In this way, the compiler might give clearer error message when we accidentally try to call these **delete**d functions.

What's more, **delete** may also be applied to non-member functions. For example, if we have a function called `isLucky` that should only take **int** and reject any implicit type conversion:

```CPP
bool isLucky(int number);	// original function

bool isLucky(char) = delete;	// reject char

bool isLucky(bool) = delete;	// reject bool

bool isLucky(double) = delete;	// reject double
```

Another trick that **delete** can perform is to prevent use of template instantiations that should be disabled. For example, suppose we have a template that works for any **T \*** pointer types. However, it should not work with **void \*** and **char \***. We could disable these two types:

```CPP
template<typename T>		// original template
void processPointer(T *ptr);

template<>
void processPointer<void>(void *) = delete;

template<>
void processPointer<char>(char *) = delete;
```

One level up, if we have a function template inside a class, and we want to disable some type **T** instantiations of that function by declaring them **private**.

However, it's not going to work because it's not allowed to give a member function template specialization a different access level from that of the main template. The below code won't compile:

```CPP
// won't compile
class Widget {
public:
  ...
  template<typename T>
  void processPointer(T* ptr) { ... }

private:
  template<>				// error!
  void processPointer<void>(void*);	// cannot have different access level
  ...
};
```

The **delete** would do the work in this case.

```CPP
class Widget {
public:
  ...
  template<typename T>
  void processPointer(T* ptr) { ... }
  ...
};

template<>
void Widget::processPointer<void>(void *) = delete;
```

In short, in C++98 we use **private** + undefined to mimic the **delete** keyword introduced in C++11. Now that we have **delete**, we should prefer **delete** over the old style.