### Item 21: Prefer **std::make_unique** and **std::make_shared** to direct use of **new**

#### Overall
- Compared to direct use of **new**, make functions eliminate source code duplication, improve exception satefy, and, for **std::make_shared** and **std::allocate_shared**, generate code that's smaller and faster.
- Situations where use of **make** functions is inappropriate include the need to specify custom deleters and a desire to pass braced initializers.
- For **std::shared_ptr**s. additional situations where make functions may be ill-advised include (1) classes with custom memory management and (2) systems with memory concerns, very large objects, and **std::weak_ptr**s that outlive the corresponding **std::shared_ptr**s.

**std::make_shared** is part of the C++11, but **std::make_unique** joined the STL as of C++14. But this should not prevent you writing a simple **make_unique** for you own C++11 project:

```CPP
template<typename T, typename... Ts>
std::unique_ptr<T> make_my_unique(Ts&&... params) {
  return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}
```

As the title indicates, in general, we should prefer the usage of **make** version of creation of smart pointers instead of the direct use of **new**.

Firstly, it avoids code duplication and yields more consistent code base:

```CPP
auto upw1(std::make_unique<Widget>());		// with make func

std::unique_ptr<Widget> upw2(new Widget);	// without make func

auto spw1(std::make_shared<Widget>());		// with make func

std::shared_ptr<Widget> spw2(new Widget);	// without make func
```

As illustrated above, with the make functions, we only need to specify the type once.

Secondly, make functions help to achieve better exception safety code. Suppose we have a function to process a `Widget` according to some priority:

```CPP
void processWidget(std::shared_ptr<Widget> spw, int priority);
int computePriority();

// potential resource leaks
processWidget(std::shared_ptr<Widget>(new Widget), computePriority());
```

It's not obvious why the above code might lead to resource leak. shared pointer should deallocate any pointer it holds. `new Widget` is guaranteed to execute before the constructor of **std::shared_ptr<Widget>**. However, consider the following order of execution:

```CPP
1. new Widget
2. computerPriority() // throw exception
3. ctor of std::shared_ptr<Widget>
```

As listed, if the 2nd step `computePriority()` throws a exception, the **std::shared_ptr** will not be able to collect the memory allocated by `new Widget`, and thus it's leaked.

Using **std::make_shared** avoids the problem:

```CPP
// no potential resource leak
processWidget(std::make_shared<Widget>(), computePriority());
```

Thirdly, a special case of **std::shared_ptr** is its control block. By using make function we could achieve 1 time memory allocation:

```CPP
// 2 memory allocations
// one for raw new, one for control block
std::shared_ptr<Widget> spw(new Widget);

// 1 memory allocation only
auto spw2(std::make_shared<Widget>());
```

However, admittedly, there are also cases where the use of make function might not be appropriate.

For example, if we want to specify a custom deletor for our smarter pointers, we are left with no choice but the direct use of **new**:

```CPP
// custom deleter
auto widgetDeleter = [](Widget* pw) { ... };

std::unique_ptr<Widget, decltype(widgetDeleter)> upw(new Widget, widgetDeleter);

std::shared_ptr<Widget> spw(new Widget, widgetDeleter);
```

The second inconvenience is that, the make function forwards parameter using "**parenthesis**" instead of "**bracket**", which means you cannot directly invoke initializer list constructor.
For example,

```CPP
// a vector of 10 elements, each equal to 20
auto spv = std::make_shared<std::vector<int>>(10, 20);

// a work-around, a vector containing 10 and 20
auto initList = { 10, 20 };
auto spv2 = std::make_shared<std::vector<int>>(initList);
```

There is another rare case that if the object you are allocating is very big and you have **std::weak_ptr** pointing to it. The control block cannot be deleted until all the **std::shared_ptr** and **std::weak_ptr** pointing to it drop to count=0. This may introduce a significant lag between when an object is destroyed and when the memory it occupied is freed.

All in all, besides some rare corner cases, we should prefer the usage of make functions instead of direct use of **new** when creating smart pointers.