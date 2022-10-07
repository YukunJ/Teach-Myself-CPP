### Item 32: Use init capture to move objects into closures

#### Overall
- Use C++14's init capture to move objects into closures.
- In C++11, emulate init capture via hand-written classes or **std::bind**

If you have a move-only object that you want to get into a closure, C++11 has not direct way  of doing that (we will talk about work-arounds later on), but C++14 has direct support for that using **init capture**.

Using an init capture makes it possible to specify

1. **the name of a data member** in the closure class generated from the lambda
2. an expression initializing that data member

Let's see an example:

```CPP
class Widget { ... };
auto pw = std::make_unique<Widget>();
... // do something with pw
auto func = [pw = std::move(pw)] { return pw->isValidated() && pw->isArchived(); };
```

To the left of the `=` is the name of the data member in the closure class you're specifying, and to the right is the initializing expression. The scope on the left is that of the closure class. The scope on the right is the same as where the lambda is defined.

Now, how about in C++11? How should we achieve move-init functionality?

Firstly, we can write a whole functor class to specify what we want:

```CPP
class IsValAndArch {
public:
  using DataType = std::unique_ptr<Widget>;

  explicit IsValAndArch(DataType&& ptr)
  : pw(std::move(ptr)) {}

  bool operator() const {
    return pw->isValidated() && pw->isArchived();
  }

private:
  DataType pw;
};
```

Secondly, if we want to stick with lambdas, the solution is

1. moving the object to be captured into a function object produced by **std::bind** 
2. giving the lambda a reference to the "captured" object

The key part of code works as follows:

```CPP
std::vector<double> data;
...
auto func = std::bind(
  [](const std::vector<double>& data)
  { /* use data */ },
  std::move(data)
  };
```

The fundamental points are:
+ It's not possible to move-construct an object into a C++11 closure, but it's possible to move construct an object into a C++11 bind object
+ Emulating move-capture in C++11 consists of move-constructing an object into a bind object, then passing the move-constructed object to the lambda by reference
+ Because the lifetime of the bind object is the same as that of the closure, it's possible to treat objects in the bind object as if they were in the closure

However, be mindful that soon in Item34 we will advocate the use of lambda over **std::bind** whenever possible. But in this item, the usage of **std::bind** might be a necessity for move-constructing a closure C++11.