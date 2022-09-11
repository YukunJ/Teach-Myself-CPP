### Item 07: Distinguish between **()** and **{}** when creating objects


#### Overall
- Braced initialization is the most widely usable initialization syntax, it prevents narrowing conversions, and it's immune to C++'s most vexing parse.
- During constructor overload resolution, braced initializers are matched to **std::initializer_list** parameters if at all possible, even if other constructors offer seemingly better matches.
- An example of where the choice between parentheses and braces can make a significant difference is creating **std::vector\<numeric type\>** with two arguments.
- Choosing between parentheses and braces for object creation inside templates can be challenging.

C++11 introduces *uniform initialization* based on braces.

Braces can be used to specify default initialization values for non-static data memebers.

```CPP
class Widget {
  ...
private:
  int x{ 0 };	// fine, x's default value is 0
  int y = 0;	// also fine
  int z(0);	// error!
}
```

On the other hand, **uncopyable** objects (e.g. **std::atomic**) may be initialized using braces or parentheses, but not using `=`:

```CPP
std::atomic<int> ai1{ 0 };	// fine

std::atomic<int> ai2(0);	// fine

std::atomic<int> ai3 = 0;	// error!
```

Thus it's easy to see why braced initialization is praised to be "uniform".

Another novel feature of braced initialization is that it prohibits implicit *narrowing conversions* among built-in types:

```CPP
double x, y, z;

int sum1{ x + y + z };	// error! sum of double may not be expressible as int

int sum2(x + y + z);	// okay (value of expression truncated to an int)

int sum3 = x + y + z;	// same okay as above
```

Another noteworthy characteristic of braced initialization is its immunity to C++'s *most vexing parse*: when developers want to default-construct an object, but inadvertently end up declaring a function instead:

```CPP
Widget w1(10);	// call ctor with argument 10

Widget w2();	// most vexing parse!
		// declares a function named w2 that return a Widget!

Widget w3{};	// call ctor with no argument
```

However, braced initialization does come with drawbacks. When there is one or more constructors declare a parameter of type **std::initializer_list**, calls using braced initialization syntax strongly prefer the overloads taking **std::initializer_list**. For example:

```CPP
class Widget {
public:
  Widget(int i, bool b);
  Widget(int i, double b);
  Widget(std::initializer_list<long double> il); // surprising behavior
  
  operator float() const;			 // convert to float
}

Widget w1(10, true);	// calls first ctor

Widget w2{10, true};	// use braces, now call std::initializer_list ctor
			// (10 and true converts to long double)

Widget w3(10, 5.0);	// calls first ctor as w1

Widget w4{10, 5.0};	// calls std::initializer_list ctor as w2

Widget w5(w4);		// calls copy ctor

Widget w6{w4};		// use braces, w4 converts to float
			// and use std::initializer_list ctor

Widget w7(std::move(w4));// calls move ctor

Widget w9{std::move(w4)};// use braces, w4 converts to float
			 // and use std::initializer_list ctor
```

Unless you declare the type in **std::initializer_list** in such a way there is no way to convert the types. For example if declare **std::initializer_list\<std::string\>** in above case.

Another quick point to catch is that, empty `{}` leads to default ctor, not a empty **std::initializer_list**.

```CPP
Widget w10{};	// calls default ctor
Widget w11({});	// calls std::initializer_list with empty list
```

The pros and cons of using `()` or `{}` in template class design are tricky questions. Only the caller can know what's the expected behavior given the argument types. In STL, for example, **std::make_unique** and **std::make_shared** use partheses internally and document this decision as part of their interfaces.








