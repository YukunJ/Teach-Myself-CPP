### Item 28: Understand reference collapsing

#### Overall
- Reference collapsing occurs in four contexts: template instantiation, **auto** type generation, creation and use of **typedef**s and alias declarations, and **decltype**.
- When compilers generate a reference to a reference in a reference collapsing context, the result becomes a single reference. If either of the original references is an lvalue reference, the result is an lvalue reference. Otherwise it's an rvalue reference.
- Universal references are rvalue references in contexts where type deduction distinguishes lvalues from rvalues and where reference collapsing occurs.

For a template like this:

```CPP
template<typename T>
void func(T&& param);
```

Since it's universal reference, the deduced type **T** will encode whether the argument passed to **param** is an lvalue or rvalue. The rule is simple: when an lvalue is passed as an argument, **T** is deduced to be lvalue reference. When an rvalue is passed, **T** is deduced to be a non-reference. Hence:

```CPP
Widget widgetFactory();

Widget w;

func(w);		// T deduced to Widget&

func(widgetFactory());	// T deduced to Widget
```

C++ doesn't allow to define reference to reference. However, this could occur during template instantiation and this leads to the rule of ***reference collapsing***:

`
If either reference is an lvalue reference, the result is an lvalue reference. Otherwise (i.e. if both are rvalue references) the result is an rvalue reference.
`

***reference collapsing*** is the technique that makes **std::forward** work. Consider a simplified version of its implementation:

```CPP
// C++14
template<typename T>
T&& forward(remove_reference_t<T>& param) {
  return static_cast<T&&>(param);
}
```

Suppose we pass an lvaue of `Widget` to **f**. **T** is thus deduced to be `Widget&`. Steps are substitution, remove reference traits, and then collapsing:

```CPP
Widget& && forward(Widget& param) {
  return static_cast<Widget& &&>(param);
}

// the above collapse into
Widget& forward(Widget& param) {
  return static_cast<Widget&>(param);
}
```

Suppose on the other hand we pass an rvalue of `Widget` to **f**. **T** is deduced to be `Widget`. And we get:

```CPP
Widget&& forward(Widget& param) {
  return static_cast<Widget&&>(param);
}
```

reference collapsing also applies to **auto**. Consider

```CPP
Widget w;

Widget widgetFactory();

auto&& w1 = w;

auto&& w2 = widgetFactory();
```

The `w1` is initialized with lvalue, so the type deduced is `Widget&`, plus the type collapsing, the final type of `w1` is `Widget&`.

The `w2` is initialized with rvalue, so the type deduced is `Widget`, plus the type collapsing, the final type of `w2` is `Widget&&`.

The third place where ***reference collapsing*** might occur is **typedef** inside a template.

```CPP
template<typename T>
class Widget {
public:
  typedef T&& RvalueRefToT;
  ...
};
```

Suppose we instantiate `Widget` with an lvalue reference type:

```CPP
// instantitate with lvalue
Widget<int&> w;

// substitute
typedef int& && RvalueRefToT;

// collapse
typedef int& RvalueRefToT;
```

The fourth place for ***reference collapsing*** is the use of **decltype**. If, during analysis of a type involving **decltype**, a reference to a reference arises, reference collapsing will kick in to eliminate it.