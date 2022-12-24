### Variadic Template

##### Source
[\<C++ Primer 5th Edition>](https://zhjwpku.com/assets/pdf/books/C++.Primer.5th.Edition_2013.pdf) Chapter 16.4 Variadic Templates

A **variadic template** is a template function or class that can take a varying number of parameters known as **parameter pack**. There are two kinds of parameter packs: A **template parameter pack** represents zero or more template parameters, and a **function parameter pack** represents zero or more function parameters.

We use ellipsis "**...**" to indicates a parameter pack. For example:

```CPP
// Args is a template parameter pack
// rest is a function parameter pack
template <typename T, typename... Args>
void foo(const T& t, const Args&... rest);
```

The **sizeof...** operator tells us how many elements are there in a parameter pack. It is a constant expression that does not evaluate its argument.

##### Writing a Variadic Function Template

Let's write a variadic version of the `print` function.

```CPP
// see variadic_print.cpp
template<typename T>
ostream& print(ostream& os, const T& t) {
  return os << t;
}

template<typename T, typename... Args>
ostream& print(ostream& os, const T& t, const Args&... rest) {
  os << t << ",";
  return print(os, rest...);
}
```

Variadic template usually works like a recursion function. Here we first provide a base version of the `print` with only one printable parameter. And then we write the variadic version of the `print`, which calls itself recursively until the parameter pack is empty.

One key observation here is that, while the varadic version requires 3 template, the recursive call `return print(os, rest...)` seems to pass in only two parameters. 

What happens is that, the first element of the pack is removed and matched to the `const T& t` part of the template, and the remaining elements stay in the pack and matched to `const Args&... rest`.

In the end when the pack is empty, both the variadic version and base version are viable. But a non-variadic template is more specialized than a variadic template, the base version is chosen for the last call. Notice, the declaration for the non-variadic base version of the `print` must be in scope where the variadic version is defined. Otherwise, the variadic function will recurse indefinitely.

##### Pack Expansion

In the last example, we already saw how to expand a parameter pack by appending "**...**" to the right side of it. Like `const Args&... rest` generates the function parameter list for `print`, and `print(os, rest...)` generates argument list for the call to `print`.

We can apply more complicated pattern matching for variadic template. For example, we might want to call `debug_rep` function on every element of the parameter pack:

```CPP
// see variadic_pattern.cpp
template <typename T>
std::string debug_prefix(const T& t) {
   return "debug:" + std::to_string(t);
}

template <typename... Args>
ostream& debug_print(ostream& os, const Args&... rest) {
    return print(os, debug_prefix(rest)...);
}
```

Here by `debug_prefix(rest)...`, we indicates that we want to apply the function `debug_prefix` to every element of the pack `rest` and expand it as the argument list to the `print` function.

##### Forwarding Parameter Packs

Variadic functions often forward their parameters to other functions, where all the type information about the parameters in the pack need to be preserved and transported. This is usually referred to as "perfect forwarding".

The general formula is as follows:

```CPP
template <typename... Args>
void fun(Args&&...args) {
  ...
  // expand both template pack Args and function pack args
  work(std::forward<Args>(args)...);
  ...
}
```
`Args&&` means each function paramter will be a rvalue reference to its corresponding argument.

Here notice we use the "pattern matching" we mentioned in last section, where both the template  pack `Args` and function argument pack `args` are zipped together to be expanded. So it's equivalent to

```CPP
std::forward<Args_1>(args_1), ..., std::forward<Args_n>(args_n);
```
