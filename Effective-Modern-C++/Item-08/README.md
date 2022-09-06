### Item 08: Prefer ***nullptr*** to **0** and ***NULL***


#### Overall
- Prefer ***nullptr*** to **0** and ***NULL***.
- Avoid overloading on integral and pointer types.

Since C++98, many programmers are used to using **0** and ***NULL*** to represent the null pointer. However, this is not perfectly OK.

The literal **0** is an **int**. If C++ finds itself looking at **0** in a context where only a pointer can be used, it will interpret **0** as a null pointer.

The same is true for ***NULL***. ***NULL*** is a macro that has uncetainty. It might be a **long** rather than **int**. But the bottomline is that neither of **0** and ***NULL*** has a pointer type.

In C++98, the major problem from this is function overloading: Passing **0** or ***NULL*** to such overloads never calls a pointer overload:

```CPP
void f(int);
void f(bool);
void f(void*);

f(0);		// calls f(int), not f(void*)
f(NULL);	// might not compile, might call f(int)
		// not never call f(void*)
```

Here comes the champion -- ***nullptr***. Its actualy type is **std::nullptr_t**, which implicitly converts to all raw pointer types, and this makes ***nullptr*** act as if it were a pointer of all types.

```CPP
f(nullptr);	// calls f(void*)
```

Moreover, using ***nullptr*** over **0** or ***NULL*** improves code clarity:

The below code remain unclear about whether result is just 0 or null pointer.

```CPP
auto result = findRecord(/* arguments */);

if (result == 0 ) {
  /* not clear result is null pointer or really int 0
}
```

Instead, if we use ***nullptr***, its intention would be much clearer.

```CPP
auto result = findRecord(/* arguments */);

if (result == nullptr ) {
  /* result is null pointer, if not found record */
}
```

***nullptr*** shines particularly when template comes into scene. Suppose we have some functions that rely on mutex locking:

```CPP
std::mutex f1m, f2m, f3m;	// mutexes for f1, f2, f3

using MuxGuard = std::lock_guard<std::mutex>

{
	MuxGuard g(f1m);
	auto result = f1(0);	// pass 0 as null ptr to f1
}

{
	MuxGuard g(f2m);
	auto result = f2(NULL);	// pass NULL as null ptr to f2
}

{
	MuxGuard g(f3m);
	auto result = f3(nullptr);	// pass nullptr as null ptr to f3
}
```

The failure to use ***nullptr*** in the first two calls in this code is sad, but th code works fine. Now we want to move this function into template:

```CPP
template<typename FuncType, typename MuxType, typename PtrType>
decltype(auto) lockAndCall(FuncType func, MuxType& mutex, PtrType ptr) {
	using MuxGuard = std::lock_guard<MuxType>;
	MuxGuard g(mutex);
	return func(ptr);
}
```

and when we try to re-run the same calling pattern as before, things happen:

```CPP
auto result1 = lockAndCall(f1, f1m, 0);		// compile error!
auto result2 = lockAndCall(f2, f2m, NULL);	// compile error!
auto result3 = lockAndCall(f3, f3m, nullptr);	// compile, good!
```

The reason why the above invocaion for `result1` and `result2` would fail is that, the type of **0** and ***NULL*** are **int**, so that's the type of the parameter `ptr` inside the template instantiation of this call to `lockAndCall`. No further conversion could be done there.s

In contrast, when ***nullptr*** is passed, the type is deduced as **std::nullptr_t**. When `ptr` is passed to `f3`, there's an implicit conversion from **std::nullptr_t** to `Widget*`, because **std::nullptr_t** implicitly converts to all pointer types.

In a nutshell, when you want to refer to a null pointer, use ***nullptr***, not **0** or ***NULL***.