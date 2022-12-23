### Item 15: Use **constexpr** whenever possible


#### Overall
- **constexpr** objects are **const** and are initialized with value known during compilation.
- **constexpr** functions can produce compile-time results when called with arguments whose values are known during compilation.
- **constexpr** objects and functions may be used in a wider range of contexts than non-**constexpr** objects and functions.
- **constexpr** is part of an object's or functions's interface.

Conceptually, **constexpr** indicates a value that's not only **const**, it's known during compilation. Values known at compilation are very previleged: they may be put into read-only memory.

Notice that **const** doesn't offer the same as **constexpr**, since **const** objects need not be initialized with known value at compilation time.


In C++, there are contexts where we need *integral constant expression*. This includes the specification of array size, integral template arguments, enumerator values, alignment specifiers, etc.

Here is some examples of what might constitute **constexpr**:

```CPP
int sz;					// non-constexpr variable

constexpr auto arraySize1 = sz;		// error! not known at compilation

std::array<int, sz> data1;		// error! same problem

constexpr auto arraySize2 = 10;		// fine, 10 is a compile-time constant

std::array<int, arraySize2> data2;	// fine arraySize2 is constexpr
```

When **constexpr** is used to decorate a function, the case varies:

- when **constexpr** function is called with parameters that's known during compilation, the value of the function will be evaluated during compilation.
- When the parameters are not known during compilation, this becomes a normal function whose value is only known in runtime.

In C++11, **constexpr** functions may only contain a single executable **return** statement. But there is trick to work around it. For example, use the **?** operator to mimic "if-else" clause and use recursion:

```CPP
constexpr int pow(int base, int exp) noexcept {
  return (exp == 0 ? 1 : base * pow(base, exp - 1));
}
```

In C++14, such constraints are lifted and we are free to write loops, etc.

**constexpr** functions are limited to take and return *literal types*. In C++11, all built-in types except **void** qualify. What's more, user-defined types may be literal because constructors and other member functions may be **constexpr**:

```CPP
class Point {
public:
  constexpr Point(double xVal = 0, double yVal = 0) noexcept
    : x(xVal), y(yVal) {}

  constexpr double xValue() const noexcept { return x; }
  constexpr double yValue() const noexcept { return y; }

  void setX(double newX) noexcept { x = newX; }
  void setY(double newY) noexcept { y = newY; }

private:
  double x, y;
};

constexpr Point midpoint(const Point& p1, const Point& p2) noexcept {
  return { (p1.xValue() + p2.xValue()) / 2,	// call constexpr
           (p1.yValue() + p2.yValue()) / 2 };	// member funcs
}

constexpr Point p1(9.4, 27.7);
constexpr Point p2(28.8, 5.3);
constexpr auto mid = midpoint(p1, p2); 
```

It's amazing to find that the midpoint `mid` could be computed during compilation and used as constant literal. 

In C++11, `setX` and `setY` cannot be declared `constexpr` because in C++11 `constexpr` is implicitly `const` and they have `void` return type. But all of the two restrictions are lifted in C++14.

Notice that `constexpr` is part of an object's or function's interface/signature. Be careful about the type compatible problem.

In conclusion, usage of **constexpr** is encouraged since they enhance runtime by moving computations to compile time.