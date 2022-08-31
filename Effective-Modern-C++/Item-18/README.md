### Item 18: Use **std::unique_ptr** for exclusive-ownership resource management 


#### Overall
- **std::unique_ptr** is a small, fast, move-only smart pointer for managing resources with exclusive ownership semantics.
- By default, resource destruction takes place via `delete`, but custom deleters can be specified. Stateful deleters and function pointers as deleters increase the size of **std::unique_ptr** objects.
- Converting a **std::unique_ptr** to a **std::shared_ptr** is easy.

**std::unique_ptr** gives exclusive ownership of the object. You cannot copy a **std::unique_ptr**, only move is allowed, which transfers the ownership from source pointer to destination pointer.

One common use of **std::unique_ptr** is as a return type from factory method. Suppose we have the following hierarchy:

```
		    Investment
			^
			|
-------------------------------------------------
|			|			|
Stock			Bond			RealEstate
```

```CPP
class Investment { ... };

class Stock: public Investment { ... };

class Bond: public Investment { ... };

class RealEstate: public Investment { ... };
```

A factory function for the `Investment` hierarchy could be declared like this:

```CPP
template<typename... Ts>		// return std::unique_ptr
std::unique_ptr<Investment>		// to an object created
makeInvestment(Ts&&... params);		// from the given args
```

By default, when a **std::unique_ptr** goes out of scope, its destructor is called to `delete` the pointer it points to. However, customerized deleter is also possible. 

In our case, suppose we want a log print before the Investment object gets destroyed. We could implement it as follows:

```CPP
/* customer deleter (a lambda expression) */
auto delInvmt = [](Investment* pInvestment) {
		makeLogEntry(pInvestment);
		delete pInvestment;
	}

template <typename... Ts>
std::unique_ptr<Investment, decltype(delInvmt)>
makeInvestment(Ts&&.. params) {
	std::unique_ptr<Investment, decltype(delInvmt)> pInv(nullptr, delInvmt);
	if ( /* a stock */) {
		pInv.reset(new Stock(std::forward<Ts>(params)...));
	} else if (/* a Bond */) {
		...
	}
	...
	return pInv;
}
```

We see that the custom deleter is passed as the second argument to **std::unique_ptr**, and we must specify its type. Thanks to `decltype`for auto type deduction. The custom deleter will always take a pointer to the object under management as its parameter.

Notice that we cannot assign a raw pointer (from `new`) directly to **std::unique_ptr**. Therefore, we use `reset` functionality to assume ownership of the object created through `new`. With each use of `new`, we use **std::forward** to perfect-forward all the arguments.

However, be noticed that custom deleter might increase the size of our **std::unique_ptr**. With default deleter, we can assume **std::unique_ptr** be of the same size as raw pointer, generally one word.

With function pointer deleter, the **std::unique_ptr** generally increase in size by one word. And for function object deleter with extensive state, it could massively increase the size of **std::unique_ptr**. That would be a good time to reconsider the design.

**std::unique_ptr** comes in two forms. One is for a single object as **std::unique_ptr\<T\>**, the other is for arrays as **std::unique_ptr<T[]>**. However, given that we have good container classes like **std::vector**, **std::array**, we should rarely use **std::unique_ptr** in its array form.

Besides, although **std::unique_ptr** express exclusive ownership of an object, it could easily covert to **std::shared_ptr**. This makes it being the return type of factory method more flexible.

```CPP
/* flexible conversion from unique_ptr to shared_ptr */
std::shared_ptr<Investment> sp = makeInvestment( arguments );
```
