### Item 27: Familiarize yourself with alternatives to overloading on universal references

#### Overall
- Alternatives to the combination of universal references and overloading include the use of distinct function names, passing parameters by lvalue-reference-to-**const**, passing parameters by value, and using tag dispatch.
- Constraining templates via **std::enable_if** permits the use of universal references and overloading together, but it controls the conditions under which compilers may use the universal reference overloads.
- Universal reference parameters often have efficiency advantages, but they typically have usability disadvantages.

In **Item27**, we discussed a few failing cases when we use overloading with universal references, particularly with special class member functions. In this item, we strive to show a few work-arounds for that situation.

1. Abandon overloading

We could just break the two `logAndAddNameIdx` overloads into `logAndAddName` and `logAndAddNameIdx`. But this won't work with the `Person` constructor case.

2. Pass by **const T&**

An alternative is to revert to C++98 and replace pass-by-universal-reference with pass-by-lvalue-reference-to-**const**. The drawback is that the design isn't as efficient as we'd prefer.

3. Pass by value

Counterintuitively, pass-by-value might not be that bad when you know you'll copy them anyway.

```CPP
class Person {
public:
  explicit Person(std::string n) // replace T&& ctor; See Item 41 for details
    : name(std::move(n)) {}

  explicit Person(int idx)
    : name(nameFromIdx(idx)) {}
  ...
private:
  std::string name;
};
```

4. Use Tag dispatch

We will reimplement `logAndAdd` to delegate to two other functions, one for integral values and one for everything else. `logAndAdd` itself serves as client api that accepts all argument types.

```CPP
// for not integral type
template<typename T>
void logAndAddImpl(T&& name, std::false_type) {
  auto now = std::chrono::system_clock::now();
  log(now, "logAndAdd");
  names.emplace(std::forward<T>(name));
}

// for integral type
void logAndAddImpl(int idx, std::true_type) {
  logAndAdd(nameFromIdx(idx));
}

// client api interface
template<typename T>
void logAndAdd(T&& name) {
  logAndAddImpl(
    std::forward<T>(name),
    std::is_integral<typename std::remove_reference<T>::type>()
  );
}
```

5. Constraining templates that take universal references

We want to "conditionally" activate the forwarding constructor. The C++14 STL trait **std::enable_if_t** allows us to do. Also, we want to disregard whether it's a reference or whether it's **const** or **volatile**. And also allow derived class instance. Overall, this leads to this charming template:

```CPP
class Person {
public:
  template<
    typename T,
    typename = std::enable_if_t<
      !std::is_base_of<Person, std::decay_t<T>>::value
      &&
      !std::is_integral<std::remove_reference<T>>::value
    >
  >
  explicit Person(T&& n) : name(std::forward<T>(n)) { ... }

  explicit Person(int idx) : name(nameFromIdx(idx)) { ... }
  ...
private:
  std::string name;
};
```

This is both beautiful and efficient.

#### Tradeoff
The first 3 techniques mentioned above specify a type for each paramter in the functions to be called. The last 2 techniques (tag dispatch and constraining template eligibility) use perfect forwarding.

As a rule of thumb, perfect forwarding is more efficient because it avoids the creation of temporary objects, but it's generally harder to write, read and debug. Compiler might give hundreds lines of error message without very clear indications of where goes wrong. A remedy is to add a small checker ourselves:

```CPP
...
{
  static_assert(
    std::is_constructible<std::string, T>::value,
    "Parameter n can't be used to construct a std::string"
  );
  ...
}
...
```

