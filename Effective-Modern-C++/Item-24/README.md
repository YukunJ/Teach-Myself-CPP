### Item 24: Distinguish universal references from rvalue references

#### Overall
- If a function template parameter has type **T&&** for a deduced type **T**, or if an object is declared using **auto&&**, the parameter or object is a universal reference.
- If the form of the type declaration isn't precisely ***type&&***, of if type deduction does not occur, ***type&&*** denotes an rvalue reference.
- Universal references correspond to rvalue references if they're initialized with rvalues. They correspond to lvalue references if they're initialized with lvalues.

In many cases, when we see **T&&**, we believe we see a rvalue that's suitable for **move**. However, this is not really true. Consider the following examples:

```CPP
void f(Widget&& param);		// rvalue reference

Widget&& var1 = Widget();	// rvalue reference

auto&& var2 = var1;		// not rvalue reference

template<typename T>
void f(std::vector<T>&& param);	// rvalue reference

template<typename T>
void f(T&& param);		// not value reference
```

In fact, "**T&&**" has two different meanings. One is the common rvalue we expect to see. Another is what's called *universal reference*, which can behave like either lvalue or rvalue.

*universal reference* can appear in two contexts. The most common is function template parameters, the other is **auto** declaration. What they have in common is the presence of **type deduction**.

When we see **T&&** without type deduction, we can be assured that it is rvalue. Just like the `void f(Widget&& param)` and `Widget&& var1 = Widget()` cases we saw above.

*universal reference* is reference, which requires initialization. If the initializer is lvalue, then it corresponds to lvalue. Similarly for rvalue correspondence.

Look at the following examples for initializer:

```CPP
template<typename T>
void f(T&& param);		// param is a universal reference

Widget w;
f(w);				// lvalue passed to f; param's type is
				// Widget& (i.e. an lvalue reference)

f(std::move(w));		// rvalue passed to f; param's type is
				// Widget&& (i.e. an rvalue reference)
```

Notice that for a reference to be *universal*, type deduction is necessary but not sufficient. It has to be exactly the form "**T&&**". This explains the case below

```CPP
template<typename T>
void f(std::vector<T>&& param);	// param is an rvalue reference for sure
```

There is other complexity that prevent you from correctly determining if a type **T&&** in template is a universal reference. Let's see a contrastive example: the STL **vector** class.

```CPP
template<class T, class Allocator = allocator<T>>
class vector {
public:
  void push_back(T&& x);
  ...
  template<class... Args>
  void emplace_back(Args&&... args);
}
```

The answer here is that, the **T&&** in **push_back** is rvalue reference, while the **Args&&...** in **emplace_back** is universal reference. Because **push_back** must exist on a particular **vector** instantiation, so by the time **push_back** is called, the type **T** is already known. On the other hand, in the **emplace_back** case, **Args...** is a different parameter template from **T**, therefore type deduction happens everytime it's invoked.

Recall we briefly mention that **auto** variable can also be *universal reference*. In specific, variables declared with **auto&&** are universal reference because type deduction happens and it's in the correct form of "**T&&**".

In the end, just a disclaimer that the word *universal reference* is a made-up term. The true underlying principle is what's called **reference collapsing**, which is not pleasant to learn at first glance. In practice, it might be easier to work with the *universal reference* concepts.