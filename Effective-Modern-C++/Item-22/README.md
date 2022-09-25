### Item 22: When using the **Pimpl** Idiom, define special member functions in the implementation file

#### Overall
- The **Pimpl** Idiom decreases build times by reducing compilation dependencies between class clients and class implementations.
- For **std::unique_ptr** **Pimpl** pointers, declare special member functions in the class header, but implement them in the implementation file. Do this even if the default function implementations are acceptable.
- The above advice applies to **std::unique_ptr**, but not to **std::shared_ptr**.

The technique of **Pimpl** (pointer to implementation) is to replace the data members of a class with a pointer to an implementation class (or struct). This helps to reduce the client code's dependencies on the class implementation and reduce compilation time.

Since C++11/14, we have the smart pointers. Therefore, the modern approach would be to use a **std::unique_ptr** to represent this Idiom. An example could be illustrated as follow:

```CPP
// in "widget.h"
class Widget {
public:
  Widget();
  ...
private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;	// smart pointer instead of raw pointer
};
```

```CPP
// in "widget.cpp"
#include "widget.h"
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl {
  std::string name;
  str::vector<double> data;
  Gadget g1, g2, g3;
};

Widget::Widget(): pImpl(std::make_unique<Impl>()) {}
``` 

Looks fine, right? The code compile, but first invocation of the class throws error:

```CPP
#include "widget.h"

Widget w; // error!
```

The problem is that, upon `w` goes out of scope, the destructor for this class `Widget` is called, which is the default one that compiler generates for us. The default deleter for **std::unique_ptr** has a static assert that check if the pointer points to a incomplete type, and we fail there.

To fix the problem, we need to make sure that at the point where the code to destroy the **std::unique_ptr\<Widget::Impl\>** is generated, `Widget::Impl` is a complete type. 

The key to do that is, declare the destructor in the header file, and put the definition in the implementation file.

Classes using the **Pimpl** Idioms are natural candidate for move support. And assuming `Gadget` supports copy operation, we should also self-provide copy operation for the class, because **std::unique_ptr** is not copyable, therefore compiler won't automatically generate copy constructor and assignment operator for us.

Put all these together, a final version of correct **Pimpl** looks like this:

```CPP
// in "widget.h"
class Widget {
public:
  Widget();
  ~Widget();
  Widget(const Widget& rhs);
  Widget(Widget&& rhs) noexcept;
  Widget& operator=(const Widget& rhs);
  Widget& operator=(Widget&& rhs) noexcept;
  ...
private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;	// smart pointer instead of raw pointer
};
```

```CPP
// in "widget.cpp"
#include "widget.h"
...
struct Widget::Impl { ... }; // as befoe

Widget::~Widget() = default;

Widget::Widget(Widget&& rhs) noexcept = default;

Widget& Widget::operator=(Widget&& rhs) noexcept = default;

Widget::Widget(const Widget& rhs) : pImpl(nullptr) {
  if (rhs.pImpl) 
    pImpl = std:::make_unique<Impl>(*rhs.pImpl);
}


Widget& Widget::operator=(const Widget* rhs) {
  if (!rhs.pImpl) {
    pImpl.reset();
  } else if (!pImpl) {
    pImpl = std::make_unique<Impl>(*rhs.pImpl);
  } else {
    *pImpl = *rhs.pImpl;
  }
  return *this;
} 
```

Notice that **std::shared_ptr** doesn't have the complicated problems we mentioned above. However, since **std::unique_ptr** is the perfect candidate for implemenation **Pimpl** Idiom, we should keep this in mind: when using **Pimpl** pattern, define special member functions in the implementation file, even if default generated ones are acceptable.