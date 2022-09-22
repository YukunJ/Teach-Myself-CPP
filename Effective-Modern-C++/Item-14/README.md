### Item 14: Declare functions **noexcept** if they won't emit exceptions

#### Overall
- **noexcept** is part of a function's interface, and that means that callers may depend on it.
- **noexcept** functions are more optimizable than non-**noexcept** functions.
- **noexcept** is particularly valuable for the move operations, **swap**, memory deallocation functions, and destructors.
- Most functions are exception-neutral rather than **noexcept**.

In C++98, exception specifications would have to be summarized type by type. Since C++11, people decide to make a distinction based on the dichotomy of whether or not the function might throw exceptions or never.

There are two different ways of declaring that a function that wont' throw exceptions:

```CPP
// C++98 style
int f(int x) throw();

// C++11 style, preferred
int f(int x) noexcept;
```

However, when deciding between the two, we prefer the latter for optimization opportunities.

Firstly, in C++98 style, when an exception leaves `f`, the call stack is unwound to caller and later on the program is terminated. On the contrary, in C++11 style, the stack is only possibly unwound before program execution is terminated. In this manner, in a **noexcept** function, optimizer need not keep the runtime stack in an unwindable state, nor must they ensure that objects in the **noexcept** function are destroyed in the inverse order of creation. Therefore,

```CPP
RetType function(params) noexcept;	// most optimizable

RetType function(params) throw();	// less optimizable

RetType function(params);		// less optimizable
```

Secondly, for some functions, the optimization chances are even bigger. Suppose we have a **std::vector** that we keep adding elements into:

```CPP
std::vector<Widget> vw;
...
Widget w;
...
vm.push_back(w);
```

When the size of the vector reaches its capacity, it will double the underlying array and copy over the elements. This is the way how vectors achieve dynamic expanding.

With the introduction of move semantics, you might want to utilize it and doing move instead of copy when expanding the vector. However, **push_back** provide strong exception safety guarantee. Move operation might violate this.

**std::vector::push_back** takes advantage of a philosophy "move if you can, but copy if you must". It will check to see if the move functions are declared **noexcept**.

However, we need to be clear on that, keeping the promise that a function won't throw exception by declaring **noexcept** is not an easy job. **noexcept** is part of the function interface. If you later on abandon it, a lot of client code might break.

Most functions are *exception-neutral*. Such functions throw no exceptions themselves, but functions they call might emit one. Therefore, most functions actually lack the **noexcept** designation.

It's worth noting that some library interface designers distinguish functions with **wide contracts** from **narrow contracts**. A function with wide contract has no precondition. A function with narrow contract has preconditions. If such preconditions are violated, the program behavior is undefined.

For example, suppose we have function that processes a **std::string** and we are sure it will not emit exception. Therefore, we should declare it **noexcept**. However, there is a precondition to this function: the length of the string passed in should be less than 32 characters long:

```CPP
// precondition: s.length() <= 32
void process(std::string) noexcept;
```

The **noexcept** is only conditionally **noexcept**, which requires the caller to obey the precondition that do not pass in a string longer than 32 characters.