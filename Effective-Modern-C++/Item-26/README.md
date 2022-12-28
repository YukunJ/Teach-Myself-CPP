### Item 26: Avoid overloading on universal references

#### Overall
- Overloading on universal references almost always leads to the universal reference overload being called more frequently that expected.
- Perfect-forwarding constructors are especially problemic, because they're typically better matches than copy constructors for non-**const** lvalues, and they can hijack derived class calls to base class copy and move constructors.

Suppose we have a program that logs every accessed name and adds it to a global data structure. We could write a version like this:

```CPP
std::multiset<std::string> names;

void logAndAdd(const std::string& name) {
  auto now = std::chrono::system_clock::now();
  log(now, "logAndAdd");
  names.emplace(name);
}

...

std::string petName("Darla");

logAndAdd(petName);

logAndAdd(std::string("Persephone"));

logAndAdd("Patty Dog");
```

This is a correct program, but not the optimal efficient one. In the first call, `petName` is passed as a lvalue so it's copied into `names`. In the second call, the parameter is bound to a rvalue, the function parameter type is lvalue, so it could possibly be moved into that. In the third call, a temporary copy of **std::string** is created out of string literal and then copied into the function call.

We could eliminate the inefficiencies in the second and third call by rewriting `logAndAdd` to take a universal reference.

```CPP
template<typename T>
void logAndAdd(T&& name) {
  auto now = std::chrono::system_clock::now();
  log(now, "logAndAdd");
  names.emplace(std::forward<T>(name));
}
```

Now, suppose some clients look up name by id. We have to also provide a kind of overload that takes index as parameter. This causes a problem for us:

```CPP
std::string nameFromIdx(int idx);	// return name corresponding to idx

// new overload
void logAndAdd(int idx) {
  auto now = std::chrono:system_clock::now();
  log(now, "logAndAdd");
  names.emplace(nameFromIdx(idx));
}

...

short nameIdx = 4;
logAndAdd(nameIdx);	// error!
```

There are two versions of `logAndAdd` overloads. The one taking a universal reference can deduce **T** to be **short&**, thus yielding exact match. This is deemed better than casting **short** to **int**. And then we all know the rest of the story: there is no constructor for **std::string** that could take a short as paramter. So the code fails.

Let's look at another failing example:

```CPP
class Person {
public:
  template<typename T>
  explicit Person(T&& n) : name(std::forward<T>(n)) {}

  explicit Person(int idx);
  
  Person(const Person& rhs); // compiler-generated copy ctor

  Person(Person&& rhs);  // compiler-generated move ctor
};

Person p("Nancy");

auto cloneOfP(p);	// this won't compile!
```

Here we are trying to create a `Person` from another `Person`, but this code won't call the copy constructor. Instead, the perfect-forwarding constructor is called which tries to initialize **std::string** from the passed-in `Person`. The reason is that, `p` is a non-**const** lvalue which gets beaten by templatized constructor. 

We could solve it if we write instead:

```CPP
const Person p("Nancy");

auto cloneOfP(p);	// good! call the copy ctor
```

The last failing example involves inheritance. The interaction among perfect-forwarding constructors and compiler-generated copy and move operations develops even more wrinkles with inheritance.

```CPP
// won't compile successfully
class SpecialPerson: public Person {
public:
  SpecialPerson(const SpecialPerson& rhs): Person(rhs) { ... } // calls base class forwarding ctor

  SpecialPerson(SpecialPerson&& rhs): Person(std::move(rhs)) { ... } // calls base class forwarding ctor

  ...
};
```

As the comments indicate, it's the base class's template forwarding constructor that's get called, because the passed in parameter is `SpecialPerson`. The code won't compile because there is no **std::string** constructor taking a `SpecialPerson`.

Overall, by the 3 failing examples, we try to convince that overloading on universal reference parameters is something we should avoid at all possible. In **Item27**, we will see some work-arounds if we want to achieve the same effect for forwarding most argument types yet able to treat some argument types in a special fashion.
