### Item 30: Familiarize yourself with perfect forwarding failure cases

#### Overall
- Perfect forwarding fails when template type deduction fails or when it deduces the wrong type.
- The kinds of arguments that lead to perfect forwarding failure are braced initializers, null pointers expressed as **0** or **NULL**, declaration-only integral **const static** data members, template and overloaded function names, and bitfields.

In C++11, "perfect forwarding" refers to that the first function passes exactly the same arguments to the second function without any modifications. Not only the objects, but also their types, whether lvalues or rvalues, whether **const** or **volatile**. For example, we may forward as follows:

```CPP
// single parameter version
template<typename T>
void fwd(T&& param) {
  f(std::forward<T>(param));
}

// variadic version
template<typename... Ts>
void fwd(Ts&&... params) {
  f(std::forward<Ts>(params)...);
}
```


However, there are some cases that ***perfect forwarding*** might fail:

1. Braced initializers

```CPP
void f(const std::vector<int>& v);

f({ 1, 2, 3 });		// fine,"{1, 2, 3}" implicitly converted to std::vector<int>

fwd({ 1, 2, 3 });	// error! doesn't compile
```

When calling `f` indirectly through the forwarding function template `fwd`, compilers no longers compare the arguments passed at `fwd`'s call site to the parameter declarations in `f`. Two reasons might be behind compiler error:

+ Compilers are unable to deduce a type
+ Compilers deduce the "wrong" type

Here in the `fwd({ 1, 2, 3 })` call above, the brace initializer is not able to deduce out **std::initializer_list** type. A simple fix as we mentioned before could be:

```CPP
auto il = { 1, 2, 3 };	// type deduced to std::initializer_list<int>
fwd(il);		// fine, perfect-forward il to f
```

2. **0** or **NULL** as null pointers

Item 8 already explains this matter that **0** and **NULL** might get deduced to be of **int** type. The fix here is easy: stick with **nullptr**.

3. Declaration-only integral **static const** and **constexpr** data members

As a general, there's no need to define integral **static const** and **constexpr** data members in classes; declarations alone suffice due to ***const propagation***.

For example:

```CPP
class Widget {
public:
  static const expr std::size_t MinVals = 28;
  ...
};
...
std::vector<int> widgetData;
widgetData.reserve(Widget::MinVals);
```

The above code is OK, however, if we switch to forwarding calls:

```CPP
void f(std::size_t val);

f(Widget::MinVals);	// fine, treated as "f(28)"

fwd(Widget::MinVals);	// error! should not link
```

The reason why it fails to link is that, reference needs to have some memory for the pointer to point to, and then, this generally requires **static const** and **constexpr** data members to be defined. Therefore, the fix here is:

```CPP
// in Widget's .cpp file
constexpr std::size_t Widget::MinVals;	// define it
```

4. Overloaded function names and template names

Assume we have the following overloads:

```CPP
int processVal(int value);

int processVal(int value, int priority);

void f(int (*pf)(int));

f(processVal);	// fine

fwd(processVal);// error! which processVal?
```

`f` knows the type of function pointers it's expecting, therefore it could pick the right one. However, `fwd` doesn't have these kind of information and get confused by the mere function name. To work around it, we could specify the function type explictly:

```CPP
using ProcessFuncType = int (*)(int);

ProcessFuncType processValptr = processVal; // specify needed signature for processVal

fwd(processValPtr);	// fine
```

5. Bitfields

This might be a rare case. By C++ rules, a non-**const** reference shall not be bound to a bit-field. To work around this, use **static_cast** again to specifically extract out the bit-field value you want and pass it to forwarding function.
