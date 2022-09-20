### Item 10: Prefer scoped **enum**s to unscoped **enum**s


#### Overall
- C++98-style **enum**s are now known as unscoped **enum**s.
- Enumerators of scoped **enum**s ae visible only within the **enum**. They convert to other types only with a cast.
- Both scoped and unscoped **enum**s support specification of the underlying type. The default underlying type for scoped **enum**s is **int**. Unscoped **enum**s have no default underlying type.
- Scoped **enum**s may always be forward-declared. Unscoped **enum**s may be forward-declared only if their declaration specifies an underlying type.

Generally declaring a name inside culy bracer limits its scope. However, this doesn't apply to C++98-style **enum**s.

```CPP
// C++98-style enum
enum Color { black, white, red };
auto white = false;	// error! white already declared in this scope
```

Their new C++11 counterpart *scoped enum*s don't leak name in this way:

```CPP
enum class Color { black, white, red };	// scoped inside Color
auto white = false;			// fine, no problem

Color c = Color::white;			// fine
auto r = Color::red;			// also fine
```

Not only does scoped **enum**s reduce namespace pollution, their enumerators are also much more strongly typed. Unscoped enumerators implicitly convert to integral types:

```CPP
// dangerous if unware of
enum Color { black, white, red };

std::vector<std::size_t> primeFactors(std::size_t x);

Color c = red;
...
if (c < 14.5) {		// compare Color to double !
  auto factors = primeFactors(c);
}
```

The above code is dangerous if you are unaware of the underlying implicit conversion. Using scoped enum will help the compiler to detect and prevent from you doing that. If that's really what you want to do, you should write explicit conversion:

```CPP
...
if (static_cast<double>(c) < 14.5) {
  auto factors = primeFactors(static_cast<std::size_t>(c));
}
``` 

In addition, scoped **enum**s has underlying default type of **int**, while unscoped **enum** doesn't have default type. It would need to infer from the detailed enumerator numbers to decide the best underlying type. This makes scoped **enum** easier to do **forward declaration**.

Both scoped and unscoped **enum**s support specification of underlying type. This will make unscoped **enum** also forward-declarable:

```CPP
enum class Status: std::uint32_t;	// underlying type for Status is std::uint32_t

enum Color: std::uint8_t;		// fwd decl for unscoped enum enabled
```

However, there is one place that unscoped **enum** might come handy. Suppose we have a tuple holding values for the name, email address and reputation:

```CPP
using UserInfo = std::tuple<std::string, std::string, std::size_t>;

UserInfo uInfo;
...
auto email = std::get<1>(uInfo);	// try grab the user email
```

However, it's tedious to remeber the corresponding index for each field. Later on in the code, you are very likely to get confused and write incorrect indexing. We could rely on **implicit conversion** of the unscoped **enum**:

```CPP
enum UserInfoFields { uiName, uiEmail, uiReputation };

UserInfo uInfo;
...
auto email = std::get<uiEmail>(uInfo);
```

The corresponding code with scoped **enum** is substantially more verbose:

```CPP
enum class UserInfoFields { uiName, uiEmail, uiReputation };

UserInfo uInfo;
...
auto email = std::get<static_cast<std::size_t>(UserInfoFields::uiEmail)>(uInfo);
```