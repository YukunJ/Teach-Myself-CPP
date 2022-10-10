### Item 34: Prefer lambdas to **std::bind**

#### Overall
- Lambdas are more readable, more expressive, and may be more efficient than using **std::bind**.
- In C++11 only, **std::bind** may be useful for implementing move capture or for binding objects with templatized function call operators.

Every since C++14, we should almost always prefer lambdas to **std::bind**. There are several considerations behind this. Let us examine them one by one.

Suppose we have a function to set up an audible alarm:

```CPP
using Time = std::chrono::steady_clock::time_point;

enum class Sound { Beep, Siren, Whistle };

using Duration = std::chrono::steady_clock::duration;

// at time t, make sound s for duration d
void setAlarm(Time t, Sound s, Duration d);
```

Further suppose at some point we decide that we'll want an alarm that will go off an hour after it's set and that will stay on for 30 seconds. The alarm sound, however, will remain undecided.

```CPP
auto setSoundL = 
  [](Sound s)
  {
    using namespace std::chrono;
   
    setAlarm(steady_clock::now() + hour(1),
             s,
             seconds(30));
  };
```

Its **std::bind** counterpart, however, is very likely to lie in a wrong implementation. We can not directly bind on the `steady_clock::now`, otherwise the expression will be evaluated when **std::bind** is called instead of the function object is called.

A correct **std::bind** version will look like this:

```CPP
auto setSoundB = 
  std::bind(setAlarm,
            std::bind(std::plus<>(),
                      std::bind(steady_clock::now),
                      hour(1)),
            _1,
            seconds(30));
```

This is our first point: Lambda version is obviously more readable.

Moreover, suppose we have another overload that takes a fourth parameter specifying the alarm volume:

```CPP
enum class Volume { Normal, Loud, LoudPlusPlus };

void setAlarm(Time t, Sound s, Duration d, Volume v);
```

The lambda version continues to work as before. However, the **std::bind** version cannot compile correctly because it doesn't know which version to bind with. To get it to compile again, we have to do:

```CPP
using SetAlarm3ParamType = void(*)(Time t, Sound s, Duration d);

auto setSoundB = 
  std::bind(static_cast<SetAlarm3ParamType>(setAlarm),
            std::bind(std::plus<>(),
                      std::bind(steady_clock::now),
                      hour(1)),
            _1,
            seconds(30));
```

In this case, the lambda version is very likely to be compiled inline, while the **std::bind** version is not likely since it involves function pointer. This is our second point about code performance optimization and efficiency.

Thirdly, using C++14 lambda it's clear whether the value is passed by value or by reference. However, there is no clear indicator in the **std::bind** case. We have to remember the rule:

+ **std::bind** always copies its arguments, but callers can achieve the effect of having an argument stored by reference by applying **std::ref** to it.

In short, compared to lambdas, code with **std::bind** is less readable, less expressive and probably less efficient.

The only cases we might use **std::bind** is to achieve move capture in C++11 and polymorphic function objects in C++11. But once we are in C++14, lambda should be our friend at hand always.