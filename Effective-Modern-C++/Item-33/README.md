### Item 33: Use **decltype** on **auto&&** parameters to **std::forward** them

#### Overall
- Use **decltype** on **auto&&** parameters to **std::forward** them.

One of the most exciting features of C++14 is *generic lambda* - that uses **auto** on their parameter specifications. Given this example lambda:

```CPP
auto f = [](auto x){ return normalize(x); };
```

this clousre class's function call operator looks like this:

```CPP
class SomeCompilerGeneratedClassName {
public:
  template<typename T>
  auto operator()(T x) const {
    return normalize(x);
  }
};
```

However, this lambda is not quite right because it always forward a lvalue to the `normalize()` function, regardless of the passed-in parameter being lvalue or rvalue. 

Ideally, we want to use universal reference with **std::forward** to perfect forwarding the parameter types to the closure:

```CPP
auto f = [](auto&& x){ return normalize(std::forward<???>(x)); };
```

However, what should be the type specification in the **std::forward**? We don't have access to the **typename T** as we usually do in template.

Recall in Item28 we explain that if an lvalue argument is passed to a universal reference parameter, the type becomes an lvalue reference. If an rvalue is passed to a universal reference parameter, the type becomes an rvalue reference. Therefore, we can use `decltype(x)` to inspect the reference type of `x`.

Item28 also explains the convention that when using **std::forward**, the type argument be an lvalue reference to indicate an lvalue and a non-reference to indicate an rvalue. However, if `x` is bound to an rvalue, `decltype(x)` will yield an rvalue reference instead of the customary non-reference.

But thanks to **reference collapsing**, an rvalue reference to an rvalue reference becomes a single rvalue reference, problem solved!

Therefore, our perfect forwarding lambda can therefore be written like that:

```CPP
// C++14
auto f = [](auto&& x)
         { return normalize(std::forward<decltype(x)>(x); };

// variadic
auto f = [](auto&&... xs)
         { return normalize(std::forward<decltype(xs)>(xs)...); };
```