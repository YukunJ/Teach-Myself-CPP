### Guideline26: Use CRTP to Introduces Static Type Categories
+ Apply the CRTP design pattern to define a compile-time abstraction for a family of related types.
+ Be aware of the limited access from the CRTP base class to the derived class.
+ Keep in mind the restrictions of the CRTP design pattern, in particular, the lack of a common base class.
+ Prefer C++20 concepts to the CRTP design pattern when possible.