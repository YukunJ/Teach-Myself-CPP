### Item 06: Use the explicitly typed initializer idiom when **auto** deduces undesired types

#### Overall
- "Invisible" proxy types can cause **auto** to deduce the "wrong" type for an initializing expression.
- The explicitly typed initializer idiom forces **auto** to deduce the type you want it to have.

Although we have been talking about the benefits of using **auto** over explicitly declaring the type, there are some tricky traps in using **auto**.

Suppose we have a function that takes a `Widget` and return a **std::vector\<bool\>** where each **bool** indicates whether the `Widget` offers a particular feature. We could write codes as below:

```CPP
std::vector<bool> features(const Widget& w);
...
Widget w;
bool highPriority = features(w)[5];	// is w of high priority?
...
processWidget(w, highPriority);		// process w in accordance with its priority
``` 

There is nothing wrong with the code. However, if we just make the seemingly innocuous change using **auto**:

```CPP
auto highPriority = features(w)[5];
...
processWidget(w, highPriority);		// undefined behavior!
```

This change leads to undefined behavior. Why? **std::vector<bool>** stores its **bool**s in packet form, one per bit. The **operator[]** is supposed to return a **T&**, but C++ doesn't allow reference to bits. Therefore, there is a **proxy** object that's returned to mimic the behavior of **bool**. The return type of **operator[]** in **std::vector<bool>** is actually **std::vector<bool>::reference**.

In the first version of code, the **proxy** object is explicitly casted to **bool** because we require the type. In the second version of code, **auto** preserves the **std::vector<bool>::reference**. The returned vector is a temporary object, therefor it goes out of scope. Therefore `highPriority` contains a dangling pointer. Evil!

In fact, the design pattern of "**Proxy**" is actually one of the most longstanding memebers of the software design pattern pantheon. Some proxy classes are designed to be apparent to clients, like **std::unique_ptr** and **std::shared_ptr**. Other proxy classes are more or less invisible.

Also in that camp some classes in C++ libraries using a technique of "*expression templates*". Consider the following code:

```CPP
Matrix sum = m1 + m2 + m3 + m4;
```

Instead of iteratively doing computation 3 times, the **operator+** for `Matrix` object might return a proxy class such as `Sum<Matrix, Matrix>` to delay evaluation. The result of above expression might actually be `Sum<Sum<Sum<Matrix, Matrix>, Matrix>, Matrix>` which can be casted to a `Matrix` object if evaluated.

The general rule is that **auto** don't go well with "invisible" proxy class. The solution is to use ***explicitly typed initializer idiom***.

For demonstration, it would go like:

```CPP
auto highPriority = static_cast<bool>(features(w)[5]);
...
auto sum = static_cast<Matrix>(m1 + m2 + m3 + m4);
...
double calcEpsilon();		// return double type, but we only need float precision
auto ep = static_cast<float>(calcEpsilon());
```