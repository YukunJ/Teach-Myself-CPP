### Item 16: Make **const** memebr functions thread safe


#### Overall
- Make **const** member functions thread safe unless you're *certain* they'll never be used in a concurrent context.
- Use of **std::atomic** variables may offer better performance than a mutex, but they're suited for manipulation of only a single variable or memory location.

Suppose we have a `Polynomial` class that could compute the root of a polynomial. And in order to avoid redunant computation, we cache the result once calculated. This computation is definitely a **const** function. We come up with the first version of code:

```CPP
class Polynomial {
public:
  using RootsType = std::vector<double>;

  RootsType roots() const {
    if (!rootsAreValid) { // if cache not valid
      ... // computation 
      rootsAreValid = true;
    }
    return rootVals;
  }

private:
  mutable bool rootsAreValid{ false };
  mutable RootsType rootVals{};
};
```

However, when two threads simultaneously call the `roots` function, they may all end up doing computations and the `rootVals` might contain duplicate roots.

```CPP
Polynomial p;
...
/* Thread 1 */                         /* Thread 2 */
auto rootsOfP1 = p.roots();            auto rootsOfP2 = p.roots();
```

The problem is that, `roots` is declared **const** but not **thread-safe**. One easiest way to address the issue is the usual one: employ a `mutex`:

```CPP
class Polynomial {
public:
  using RootsType = std::vector<double>;

  RootsType roots() const {
    std::lock_guard<std::mutex> g(m);	// lock mutex
    if (!rootsAreValid) { 		// if cache not valid
      ... // computation 
      rootsAreValid = true;
    }
    return rootVals;
  }

private:
  mutable std::mutex m;
  mutable bool rootsAreValid{ false };
  mutable RootsType rootVals{};
};
```

However, sometimes the `mutex` might be an overkill. For example, for a simple counter, a **std::atomic** counter would be a less expensive way to do the job:

```CPP
class Point {
public:
  ...
  double distanceFromOrigin() const noexcept {
    ++callCount;
    ...
    return std::hypot(x, y);
  }
private:
  mutable std::atmoic<unsigned> callCount{ 0 };
  double x, y;
}
```

Remember that both **std::mutex** and **std::atomic** are uncopyable and unmovable, so the class containing them is also uncopyable and unmovable.

Since operations on **std::atmoic** are less expensive than **std::mutex** acquisition and release, you may tempte to use **std::atmoic** heavily. However, it is not that universally applicable.

Let's look at an counter-example:

```CPP
class Widget {
public:
  ...
  int magicValue() const {
    if (cacheValid) return cachedValue;
    else {
      auto val1 = expensiveComputation1();
      auto val2 = expensiveComputation2();
      cachedValue = val1 + val2;	// first compute and assign
      cacheValid = true;		// then validate cache
      return cachedValue;
    }
  }
private:
  mutable std::atmoic<bool> cacheValid{ false };
  mutable std::atmoic<int> cachedValue;
};
```

The problem with the above code is that two threads might enter into the `else` branch at the same time and doing duplicate computations. You might think: "Ok, then I just first validate the cache then."

```CPP
class Widget {
public:
  ...
  int magicValue() const {
    if (cacheValid) return cachedValue;
    else {
      auto val1 = expensiveComputation1();
      auto val2 = expensiveComputation2();
      cacheValid = true;		// first validate cache
      cachedValue = val1 + val2;	// then compute and assign
      return cachedValue;
    }
  }
private:
  mutable std::atmoic<bool> cacheValid{ false };
  mutable std::atmoic<int> cachedValue;
};
```

Unfortunately, this code is still wrong. One thread might first validate the cache and in the middle of comptuation, while the other thread see the cache is valid and grab a undefined value from `cachedValue`. This is certainly leading to disasters.

The proper solution in this case is **std::mutex** to protect the whole critical session.

The lesson is that, for a single variable or memory location requiring synchronization, use of a **std::atomic** is adequate, but once you get to two or more variables or memory locations that require manipulation as a unit, you should reach for a **std::mutex**.