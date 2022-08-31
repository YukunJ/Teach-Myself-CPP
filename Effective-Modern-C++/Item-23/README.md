### Item 23: Understand **std::move** and **std::forward**

#### Overall
- **std::move** performs an unconditional cast to an rvalue. In and of itself, it doesn't move anything.
- **std::forward** casts its argument to an rvalue only if that argument is bound to an rvalue.
- Neither **std::move** nor **std::forward** do anything at runtime.
- Move requests on **const** objects are treated as copy requests.

There are confusions around **std::move** and **std::forward**. They actually don't move or forward anything. They are closer to "cast" functionalities.

**std::move** unconditionally casts its argument to an rvalue, while **std::forward** performs this cast if the parameter is bound to an rvalue.

A simple implementation of **std::move** could be illustrated as follows:

```CPP
/* C++11 version */
template<typename T>
typename remove_reference<T>::type&& move(T&& param) {
	using ReturnType = typename remove_reference<T>::type&&;
	return static_cast<ReturnType>(param);
}
```

```CPP
/* C++14 version */
template<typename T>
decltype(auto) move(T&& param) {
	using ReturnType = remove_reference_t<T>&&;
	return static_cast<ReturnType>(param);
}
```

notice that if type **T** happens to be a lvalue reference, **T&&** would be a lvalue reference. That's why we need the type trait **std::remove_reference** here.

There is one caveat that rvalues are typically *good candidates* for moving, not *100%* guaranteed. 

Consider the following snippet:

```CPP
class Annotation {
public:
    explicit Annotation(const std::string text)
        : value(std::move(text)) { ... }
    ...
private:
    std::string value;
};
}
```

This class constructor takes in a **std::string** as parameter and want to copy it over. The code compiles, links and runs. However, what's really invoked is actually the **copy constructor** instead of **move constructor** for the **std::string**.

Why? *text* is a const **std::string**, therefore after the **move**, it's casted to a rvalue **const std::string**.

Consider how **std::string** constructor is called:

```CPP
class string {
public:
    ...
    string(const string& rhs); // copy ctor
    string(string&& rhs); // move ctor
    ...
};
```

Since move constructor takes an rvalue reference to a **non-const std::string**, it cannot be invoked. However, for the copyt constructor, an lvalue-reference-to-**const** is permitted to bind to a **const** rvalue.

Therefore, the lesson here is that, don't declare **const** if we want it to be moved. And the only thing we know for sure is that, after apply **std::move**, we get an rvalue.

---

**std::forward** is similar to **std::move** that it casts parameters to rvalue, but only conditionally. It casts to an rvalue only if its arguments was initialized with an rvalue.

The typical scenario is a function template that taking a universal reference parameter and passing it to another function.

Consider a logging system:

```CPP
void process(const Widget& lvalArg); // process lvalue
void process(Widget&& rvalArg); // process rvaleue

template<typename T>
void logAndProcess(T&& param) {
	auto now = std::chrono::system_clock::now();
	makeLogEntry("Calling 'process'", now);
	process(std::forward<T>(param));
}

Widget w;
logAndProcess(w); // call with lvalue
logAndProcess(std::move(w)); // call with rvalue
```
