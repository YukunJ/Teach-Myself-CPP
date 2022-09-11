### Item 02: Understand ***auto*** type deduction


#### Overall
- ***auto*** type deduction is usually the same as template type deduction, but ***auto*** type deduction assumes that a braced initializer represents a **std::initializer_list**, and template type deduction doesn't.
- ***auto*** in a function return type or a lambda parameter implies template type deduction, not ***auto*** type deduction.


In Item-01 we have already seen how template type deduction works. In a lot of cases, the same applies to ***auto*** keyword type deduction. For example, if you write:

```CPP
auto x = 27;
const auto cx = x;
const auto& rx = x;
```

Conceptually, you can think there are some templates generated and deduce type for the ***auto*** keyword as follows:

```CPP
template<typename T>
void func_for_x(T param);

func_for_x(27);

template<typename T>
void func_for_cx(const T param);

func_for_cx(x);

template<typename T>
void func_for_rx(const T& param);

func_for_rx(x);
```

If you recall, there are three rules to be applied in template type deduction:

+ Case1: the type specifier is a pointer or reference, but not a universal reference
+ Case2: the type specifier is a universal reference
+ Case3: the type specifier is neither a pointer nor a reference

We already see case 1 and 3 above. The case 2 would work as you expect:

```CPP
auto&& uref1 = x;	// x is int and lvalue
			// so uref1's type is int&

auto&& uref2 = cx;	// cx is const int and lvalue
			// so uref2's type is const int&

auto&& uref3 = 27;	// 27 is int and rvalue
			// so uref3's type is int&&
```

However, there is one exception we need to be careful here. It is about the usage of "`{`" and "`}`" bracket. It might be surprising to know that ***auto*** deduce different types for the first two cases and second two cases below:

```CPP
auto x1 = 27;	// type is int, value is 27
auto x2(27);	// same as above

auto x3 = {27};	// type is std::initializer_list<int>, value is 27
auto x4{27};	// same as above
```

The speicial rule goes as follow: when the initializer for an ***auto***-declared variable is enclosed in braces, the deduced type is a **std::initializer_list**. If such a type cannot be deduced, the code will be rejected:

```CPP
auto x5 = {1, 2, 3.0};	// error! cannot deduce T for std::initializer_list<T>
```

There are two kinds of type deduction taking place. Firstly, since we are using ***auto*** with brackets `{}`, `x5`'s type must be deduced to a **std::initializer_list**. But **std::initializer_list** is a template, instantiations are **std::initializer_list\<T\>** fo some **T**. This **T**'s type must also be successfully deduced.

In C++14, ***auto*** is allowed to indicate a function's return type. However, these uses of ***auto*** employ *template type deduction*, not ***auto*** type deduction. So a function with an ***auto*** return type that return a braced initializer won't compile:

```CPP
auto createInitList() {
  return { 1, 2, 3 };	// error: cannot deduce type for { 1, 2, 3 }
}

auto resetV = [&v](const auto& newValue) { v = newValue; };	// C++14
...
resetV({ 1, 2, 3 }); 	// error: cannot deduce type for { 1, 2, 3 }
```




