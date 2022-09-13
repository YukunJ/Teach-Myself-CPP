### Item 03: Understand ***decltype***


#### Overall
- ***decltype*** almost always yields the type of a variable or expression without any modifications.
- For lvalue expression of type T other than names, ***decltype*** always reports a type of **T&**.
- C++14 supports **decltype(auto)**, which, like **auto**, deduces a type from its initializer, but it performs the type of deduction using the ***decltype*** rules.

Given a name or an expression, ***decltype*** tells you the names's or the expression's type. Typically, it will give you what you expect to see:

```CPP
const int i = 0;		// decltype(i) is const int

bool f(const Widget& w);	// decltype(w) is const Widget&
				// decltype(f) is bool(const Widget&)

struct Point {			// decltype(Point::x) is int
  int x, y;
};

std::vector<int> v;		// decltype(v) is vector<int>
...
if (v[0] == 0) ... 		// decltype(v[0]) is int&
```

However, there could be some edge cases. For example, **std::vector<bool>**'s operator[] return **bool** not a **bool&**. We need some modifications with template. A first version in C++11 could be:

```CPP
// works, but need refinement
template<typename Container, typename Index>
auto authAndAccess(Container& c, Index i)
  -> decltype(c[i])
{
  authenticateUser();
  return c[i];
}
```

The above is C++11's *trailing return type* syntax. It allows to use the parameter arguments as part of return type deduction. 

In C++14, we could omit the trailing return type and just use the **auto**. However, it's easy to write an incorrect C++14 version of the above code:

```CPP
// incorrect C++14!
template<typename Container, typename Index>
auto authAndAccess(Container& c, Index i) {
  authenticateUser();
  return c[i];
}

std::vector<int> d;
...
authAndAccess(d, 5) = 10;	// this won't compile!
```

Here, **d[5]** returns an **int&**, but **auto** type deduction strips off the reference, thus yielding a return type of **int**. Here comes the recipe of **decltype(auto)**: **auto** specifies that the type is be deduced, and **decltype** say that **decltype** rules should be used during the deduction.

```CPP
// C++14 works. But still needs refinement!
template<typename Container, typename Index>
decltype(auto) authAndAccess(Container& c, Index i) {
  authenticateUser();
  return c[i];
}
```

Now **authAndAccess** truly return whatever **c[i]** returns, **T&** or **T**.

The use of **decltype(auto)** is not limited to function return types. It can also be convenient for declaring variables when you want to apply the **decltype** type deduction rules to initializing expressions:

```CPP
Widget w;

const Widget& cw = w;

auto myWidget1 = cw;		// myWidget1's type is Widget

decltype(auto) myWidget2 = cw;	// myWidget2's type is const Widget&
```

One thing we have not touched up is that, currently the container is passed by *lvalue-reference-to-non-const*, it would be good if rvalue could also be passed, which is a rare case though. We could define an overload version for rvalue container, which is too cumbersome to maintain.

This is the time we need **universal reference**.

```CPP
// final C++14 version
template<typename Container, typename Index>
decltype(auto) authAndAccess(Container&& c, Index i) {
  authenticateUser();
  return std::forward<Container>(c)[i];
}

// final C++11 version
template<typename Container, typename Index>
auto authAndAccess(Container&& c, Index i) 
  -> decltype(std::forward<Container>(c)[i])
{
  authenticateUser();
  return std::forward<Container>(c)[i];
}
```

There are also some tricky situation with **decltype(auto)** usage. But typically we won't be facing that. Just one case for curiosity:

```CPP
decltype(auto) f1() {
  int x = 0;
  ...
  return x;		// decltype(x) is int, so f1 returns int
}

// dangling reference to local variable!
decltype(auto) f2() {
  int x = 0;
  ...
  return (x);		// decltype((x)) is int&, so f1 returns int&
}
```