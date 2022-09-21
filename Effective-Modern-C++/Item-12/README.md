### Item 12: Declare overriding functions **override**

#### Overall
- Declare overriding functions **override**.
- Member function reference qualifiers make it possible to treat lvalue and rvalue objects (**\*this**) differently.

In C++, we can achieve polymorphism by declaring functions **virtual** in base class and override them in the derived class. However, it's very easy to overlook some details and make it the wrong way. The compiler might not even warn you about it. 

Let's look at some bad examples of failing to override:

```CPP
class Base {
public:
  virtual void mf1() const;
  virtual void mf2(int x);
  virtual void mf3() &;
  void mf4() const;
};

class Derived: public Base {
public:
  virtual void mf1();
  virtual void mf2(unsigned int x);
  virtual void mf3() &&;
  virtual void mf4() const;
}
```

None of the 4 functions above declared in **Derived** class successfully override their counterpart in **Base** class. Why?

1. `mf1` is declared **const** in **Base**, yet not in **Derived**.
2. `mf2` takes an **int** in **Base**, but an **unsigned int** in **Derived**.
3. `mf3` is lvalue-qualified in **Base**, but rvalue-qualified in **Derived**.
4. `mf4` isn't declared **virtual** in **Base**.

The general rules for overriding to occur must be met:

+ The base class function must be virtual.
+ The base and derived function names must be identical (except in the case of destructors).
+ The parameter types of the base and derived functions must be identical.
+ The **const**ness of the base and derived functions must be identical.
+ The return types and exception specifications of the base and derived functions must be compatibile.
+ The function's *reference qualifier* must be identical.

Since C++11, the **override** keyword provides the ability for programmer to tell compiler explicitly that you would like to override a function. If such override is not successful, i.e. if you overlook some of the requirements mentioned above, compiler will explictly reject and tell you where the problem is.

Therefore, the lesson for us here is that, whenever you intend to override a virtual function in Base class, use the **override** keyword for satefy and get compiler help.

We will say one more thing about the *reference qualifier*. This is to distinguish between whether the object (**\*this**) is lvalue object or rvalue object. It's not very common usage, but it does arise sometime. For example:

```CPP
class Widget {
public:
  using DataType = std::vector<double>;
  ...
  DataType& data() & {		// for lvalue Widgets, return lvalue
    return values;
  }
  
  DataType&& data() && {	// for rvalue Widgets, return rvalue
    return std::move(values);
  }
  ...
private:
  DataType values;
};

auto vals1 = w.date();		 // call lvalue overload, copy-construct vals1

auto vals2 = makeWidget().data();// call rvalue overload, move-construct vals2
```