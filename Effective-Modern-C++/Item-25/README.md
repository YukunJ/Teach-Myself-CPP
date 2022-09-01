### Item 25: Use **std::move** on rvalue references, **std::forward** on universal references

#### Overall
- Apply **std::move** to rvalue references and **std::forward** to universal references the last time each is used.
- Do the same thing for rvalue references and universal references being returned from functions that return by value.
- Never apply **std::move** or **std::forward** to local objects if they would otherwise eligible for the return value optimization (RVO).

rvalue references should be unconditionally cast to rvalues (via **std::move**), and universal references should be conditionally cast (via **std::forward**). A good example is illustrated below:

```CPP
class Widget {
public:
  Widget(Widget&& rhs) :	// rhs is rvalue reference
    name(std::move(rhs.name)), p(std::move(rhs.p)) { ... }
  ...
  template<typename T>
  void setName(T&& newName) { 	// newName is universal reference
    name = std::forward<T>(newName);
  }
private:
  std::string name;
  std::shared_ptr<SomeDataStructure> p;
}
```

And it's dangerous to use **std::move** on universal reference. When we pass local variable to that function, the local variable's value might be moved and we won't know that. This will lead to undefined behavior later in the program.

One might argue that, for the above example we could provide for overloaded version of the `setName` function wo avoid the case:

```CPP
void setName(const std::string& newName);	// set from const lvaue

void setName(std::string&& newName);		// set from rvalue
```

There are 3 main drawbacks associated with this:

1. There would be more code to write and maintain.
2. There will be performance penality. Consider `w.setName("Adela Novak")`. In the template version, the string literal "Adela Novak" would be passed to the assignment operator for **std::string** inside `w`, the `name` field would be assigned directly from the string literal. In the 2 overloaded functions version, we will execute one **std::string** constructor (to create the temporary), one **std::string** move assignment operator (to move `newName` to `w.name`) and one destructor for **std::string** (to destroy the temporary). This definitely incur a runtime cost.
3. To overload is not scalable in design. There might be `n` parameters to be passed in, each could be a rvalue or lvalue. To overload all of them we would need `2^n` functions. And some functions take unlimited number of parameters, for example:

```CPP
template <class T, class... Args>		// from C++14 Standard
unique_ptr<T> make_unique(Args&&... args);
```

In those cases, universal reference is the only solution to be generic about rvalue and lvalue.

And keep in mind that, when we use the object bound to rvalue/universal reference more than once in a function, we will only apply **std::move** or **std::forward** the last appearance of it, because we want to make sure it's not moved away until we are done with it. The following is an example on this point:

```CPP
template<typename T>
void setSignText(T&& text) {
  sign.setText(text); 				// use text, don't modify it
  auto now = std::chrono::system_clock::now();
  signHistory.add(now, std::forward<T>(text)); // conditionally cast text to rvalue
}
```

If we're in a function that returns by *value* and we're returning an object bound to rvalue/universal references, we will benefit by apply **std::move** or **std::forward** when we return the reference. We will benefit because the reference is moved into return value location instead of copying. Consider a matrix addition example:

```CPP
/* version1: good, use move */
Matrix operator+(Matrix&& lhs, const Matrix& rhs) {	// by-value return
  lhs += rhs;
  return std::move(lhs);
}

/* version2: not so good, copy called */
Matrix operator+(Matrix&& lhs, const Matrix& rhs) {
  lhs += rhs;
  return lhs;
}
```

However, things are different when it comes to return-by-value a local object. This is a compiler optimization called "**Return Value Optimization(RVO)**".

The priniciple goes as follows:

- The compiler may elide the copying (or moving) of a local object in a function that return by value if:
- 1. the type of the local object is the same as that returned by the function
- 2. the local object is what's being returned

In this case, if we apply **std::move**, it will violate the second point because we are return the reference, not the object. This will disable **RVO**. This could be exemplified in the following example:

```CPP
/* seemingly inefficient but actually good version with RVO */
Widget makeWidget() {
  Widget w;
  ...
  ...
  return w; // RVO, construct w in return value address
}

/* seemingly smart but actually bad, disable compiler optimization */
Widget makeWidget() {
  Widget w;
  ...
  ...
  return std::move(w);
}
```











