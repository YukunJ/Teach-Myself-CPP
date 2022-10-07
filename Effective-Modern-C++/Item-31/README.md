### Item 31: Avoid default capture modes

#### Overall
- Default by-reference capture can lead to dangling references.
- Default by-value capture is susceptible to dangling pointers (especially **this**), and it misleadingly suggests that lambdas are self-contained.

Lambda expression is a powerful tool in modern C++. There are two default capture modes in C++11: by-reference and by-value. Default by-reference capture can lead to dangling references. Default by-value capture lures you into thinking you're immune to that problem and think your closures are self-contained.

Let's start with by-reference capture:

```CPP
using FilterContainer = std::vector<std::function<bool>(int)>>;

void addDivisorFilter() {
  auto calc1 = computeSomeValue1();
  auto calc2 = computeSomeValue2();
  auto divisor = computeDivisor(calc1, calc2);

  filters.emplace_back(	// ref to divisor will dangle!
    [&](int value) { return value % divisor == 0; }
  );
}
```

The lambda refers to the local variable **divisor**, but that variable ceases to exist when `addDivisorFilter()` returns.

It's fine to use capture by-reference mode when you know a closure will be used immediately and won't be copied. Otherwise, the fix here is to use by-value capture mode:

```CPP
filters.emplace_back(
  [=](int value) { return value % divisor == 0; }
);
```

This suffices for this example. However, don't think that capture by-value will save all of your problems. Let's see the next example:

```CPP
class Widget {
public:
  ...
  void addFilter() const;

private:
  int divisor;
};

void Widget::addFilter() const {
  filters.emplace_back(
    [=](int value) { return value % divisor == 0; }
  );
}
```

This is **WRONG**. Why? Captures only apply to non-**static** local variables visible in the scope where the lambda is created. The explanation hinges on the imlicit use of a raw pointer: **this**. Compiler actually treats the code as follows:

```CPP
void Widget::addFilter() const {
  auto currentObjectPtr = this;

  filters.emplace_back(
    [currentObjectPtr](int value)
    { return value % currentObjectPtr->divisor == 0; }
  );
}
```

Therefore, this lambda is tied to the lifetime of the **Widget** whose **this** pointer they contain a copy of. This particular problem can be solved by making a local copy of the data member you want to capture and then capturing the copy:

```CPP
void Widget::addFilter() const {
  auto divisorCopy = divisor;

  filters.emplace_back(
    [=](int value)
    { return value % divisorCopy == 0; }
  );
}
```

In C++14, a better way to capture a data member is to use generalized lambda capture:

```CPP
void Widget::addFilter() const {
  filters.emplace_back(
    [divisor = divisor](int value)
    { return value % divisor == 0; }
  );
}
```