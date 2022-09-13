### Item 04: Know how to view deduced types


#### Overall
- Deduced types can often be seen using IDE editors,compiler error messages, and the Boost TypeIndex library.
- The results of some tools may be neither helperful nor accurate, so an understanding of C++'s type deduction rules remains essential.

There are quite many ways to help us view the result of a type deduction in C++ program.

##### IDE Editors
Code editors in IDEs often show the type of program entities (e.g. variables, parameters, functions, etc.)

For this to work, the code must more or less be in compilable state, because there is C++ compiler running inside the IDE to perform type deduction.

##### Compiler Diagnostics
An efficient way to get the compiler to show the type is to, make use of the types that lead to compilation problems. For example:

```CPP
template<typename T>		// declaration only for TD:
class TD;			// TD == "Type Displayer"

const int theAnswer = 42;
auto x = theAnswer;
auto y = &theAnswer;

TD<decltype(x)> xType;		// elicit errors containing
TD<decltype(y)> yType;		// x's and y's types
```

If you try to compile the above code, the error messages will tell you a lot of information:

```CPP
error: aggregate 'TD<int> xType' has incomplete type and cannot be defined
error: aggregate 'TD<const int *> yType' has incomplete type and cannot be defined
```

##### Runtime Output

A very basic usage for showing the type in runtime is through **typeid** and **name**:

```CPP
std::cout << typeid(x).name() << '\n';
std::cout << typeid(y).name() << '\n';
```

However, unfortunately this approach is not always reliable. **std::type_info::name** mandates that the type be treated as if it had been passed to a template function as a by-value parameter. This implies that, by Item-01, the reference-ness and const-ness of the parameter will be removed.

##### Boost

Boost library provides a more reliable runtime type deduction show off -- its **TypeIndex** library.

```CPP
#include <boost/type_index.hpp>

template<typename T>
void f(const T& param) {
  using std::cout;
  using boost::typeindex::type_id_with_cvr;

  // show T
  cout << "T =    "
       << type_id_with_cvr<T>().pretty_name() << '\n';

  // show param
  cout << "param =    "
       << type_id_with_cvr<decltype(param)>().pretty_name() << '\n';
}
```

The way this works is that the function template **boost::typeindex::type_id_with_cvr** takes a type argument and doesn't remove **const**, **volatile** or reference qualifier.