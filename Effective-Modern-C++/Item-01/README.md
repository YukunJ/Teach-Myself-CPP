### Item 01: Understand template type deduction


#### Overall
- During template type deduction, arguments that are references are treated as non-references, i.e., their reference-ness is ignored.
- When deducing types for universal reference parameters, lvalues arguments get special treatment.
- When deducing types for by-value parameters, **const** and/or **volatile** arguments are treated as non-**const** and non-**volatile**.
- During template type deduction, arguments that are array or function names decay to pointers, unless they're used to initialize references.

Template type deduction is great thing to use: programmers just plug in values and the template works perfectly. Many even don't know the exact type the template is initiated with.

But with the introduction of C++11/14, **auto** and **decltype** make thing a bit harder.

We will base our discussion in the following based on such a template:

```CPP
template<typename T>
void f(ParamType param);

f(expr);		// call f with some expression
```

##### Case1: ***ParamType*** is a Reference or Pointer, but not a Universal Reference

The rules are:

1. If ***expr***'s type is reference, ignore the reference part.
2. Then pattern-match ***expr***'s type against ***ParamType*** to determine **T**.

For example,

```CPP
template<typename T>
void f(T& param);

int x = 27;
const int cx = x;
const int& rx = x;

f(x);		// T is int, param's type is int&
f(cx);		// T is const int, param's type is const int
f(rx);		// T is const int, param's type is const int
``` 

If we change the type of ***f***'s parameter from **T&** to **const T&**, things change a little because **const** no longer needs to be deduced as part of **T**:

```CPP
template<typename T>
void f(const T& param);

int x = 27;
const int cx = x;
const int& rx = x;

f(x);		// T is int, param's type is const int&
f(cx);		// T is int, param's type is const int&
f(rx);		// T is int, param's type is const int&
```

If ***param*** were a pointer (or a pointer to **const**) instead of a reference, things work essentially the same:

```CPP
template<typename T>
void f(T* param);

int x = 27;
const int *px = &x;

f(&x);		// T is int, param's type is int*
f(px);		// T is const int, param's type is const int*
```

##### Case2: ***ParamType*** is a Universal Reference

To know about what is "Universal Reference", see Item 24.

The rules are:

1. If ***expr*** is an lvalue, both **T** and ***ParamType*** are deduced to be lvalue reference.
2. If ***expr*** is an rvalue, the "normal" (case1) rule applies.

For example:

```CPP
template<typename T>
void f(T&& param);		// param is now a universal reference

int x = 27;
const int cx = x;
const int& rx = x;

f(x);		// x is lvalue, so T is int&, param's type is also int&

f(cx);		// cx is lvalue, so T is const int&, param's type is also const int&

f(rx);		// rx is lvalue, so T is const int&, param's type is also const int&

f(27);		// 27 is rvalue, so T is int, param's type is int&&
```

##### Case3: ***ParamType*** is Neither a Pointer nor a Reference

In this case, we're dealing with pass-by-value. The rules are:

1. As before, if ***expr***'s type is a reference, ignore the reference part.
2. If, after ignoring ***expr***'s reference-ness, ignore any **const** and **volatile** as well.

The example goes as follows:

```CPP
template<typename T>
void f(T param);		// param is now pass-by-value

int x = 27;
const int cx = x;
const int& rx = x;
const char* const ptr =  "Fun with pointers";

f(x);		// T's and param's type are int
f(cx);		// T's and param's type are int
f(rx);		// T's and param's type are int
f(ptr);		// T's and param's type are const char*
```

##### Rare Case: Array and Function Arguments' Decay

There are some rare cases, just good to know.

When we pass an array's name to a function as parameters, it actually decays into the pointer pointing to the first element in the array. Therefore,

```CPP
template<typename T>
void f(T param);			// pass-by-value

const char name[] = "J. P. Briggs";	// name's type is const char[13]

f(name); 				// T is deduced as const char*
```

But it can really deduce out the array type if we add reference to arrays!
```CPP
template<typename T>
void f(T& param);

f(name);				// pass array to f
```

Besides, function also decays into function pointers.

```CPP
void someFunc(int, double);	// type is void(int, double)

template<typename T>
void f1(T param);

template<typename T>
void f2(T& param);

f1(someFunc);			// param deduced as ptr-to-func; type is void (*)(int, double)
f2(someFunc);			// param deduced as ref-to-func; type is void (&)(int, double)
```