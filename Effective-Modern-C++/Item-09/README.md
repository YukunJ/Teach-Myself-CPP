### Item 09: Prefer alias declarations to ***typedef***s


#### Overall
- ***typedef***s don't support templatization, but alias declarations do.
- Alias templates avoid the "**::type**" suffix and, in templates, the "**typename**" prefix often required to refer to ***typedef***s.
- C++14 offers alias templates for all the C++11 type traits transformations.

In C++, we don't want to write a long very type prefix. Therefore, we want to create a synonym for type. In C++98 we have the keyword ***typedef*** and since C++11 we could apply alias **using**:

```CPP
// FP is a synonym for a pointer to a function taking an int and
// a const std::string& and return nothing
typedef void (*FP)(int, const std::string&);	// typedef

using FP = void (*)(int, const std::string&);	// alias declaration
```

The advocate here is that, we should prefer alias declaratio **using** over old ***typedef***. Why? Let's see an example. Suppose we want to define a synonym for a linked list that uses a custom allocator:

```CPP
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>;

MyAllocList<Widget> lw;
``` 

With a ***typedef***, we have to create to from scratch:

```CPP
template<typename T>
struct MyAllocList {
  typedef std::list<T, MyAlloc<T>> type;
};

MyAllocList<Widget>::type lw;
```

It gets worse when template comes into play. You have to precede the ***typedef*** name with ***typename*** keyword:

```CPP
template<typename T>
class Widget {
private:
  typename MyAllocList<T>::type list;
  ...
};
```

If **MyAllocList** is defined through alias template, this need for **typename** vanishes as well as the "**::type**" suffix:

```CPP
template<typename T>
class Widget {
private:
  MyAllocList<T> list;	// no "typename", no "::type"
  ...
};
```

In C++11, there are tools to perform type transformations in the form of ***type traits***, a collection of templates inside the header <*type_traits*>. For historical reasons, there are defined using ***typedef***. 

Given a type **T** to which you'd like to apply a transformation, the resulting type is **std::transformation\<T\>::type**. For example:

```CPP
std::remove_const<T>::type		// yields T from const T

std::remove_reference<T>::type		// yield T from T& and T&&

std::add_lvalue_reference<T>::type	// yields T& from T
```

Thanks to C++14, there have been alias templates for each such transformation traits in C++11. For each C++11 transformation **std::transformation\<T\>::type**, there is a corresponding C++14 alias template named **std::transformation_t\<T\>**. For example:

```CPP
std::remove_const<T>::type		// C++11: yields T from const T
std::remove_const_t<T>			// C++14 equivalent

std::remove_reference<T>::type		// C++11: yield T from T& and T&&
std::remove_reference_t<T>		// C++14 equivalent

std::add_lvalue_reference<T>::type	// C++11: yields T& from T
std::add_lvalue_reference_t<T>		// C++14 equivalent
```