### Item 29: Assume that move operations are not present, not cheap, and not used

#### Overall
- Assume that move operations are not present, not cheap and not used.
- In code with known types or support for move semantics, there is no need for assumptions.

Move semantics is arguably one of the most important features in C++11. In good cases, it makes "copying" as cheap as copying pointers. However, the lesson in this item is that, you don't want to always 100% confident rely on move semantics.

Firstly, many types fails to support move semantics. If the legacy code base you are working with makes no modifications for C++11, move semantics is probably not going to help you. Recall although compiler might generate default move operations for the class, it's conditioned upon that the class declares no copy operations, move operations or destructors. Data members or base classes of types that have disabled moving will also suppress compiler-generated move operations.

Secondly, sometimes the move operations are just not as cheap as you might imagine. Consider **std::array**, it has an underlying array that store the elements contained. Therefore, move operations run in **O(n)** time instead of **O(1)** time, not as easy as just copying over a pointer. (Although arguably if the object it contains supports move semantics, it might do some good).

Thirdly, even if the type supports move operations, it might not be usable. Some seemingly sure-safe move situations can end up making copies. Some **STL** containers offer strong exception satefy guarantee. The underlying copy operations may be replaced with move operations only if the move operations are known to not throw.

Therefore, in summary, there are several situations in which C++11's move semantics do you no good:

+ **No move operations**: The object to be moved from fails to offer move operations. The move request therefore becomes a copy request.

+ **Move not faster**: The object to be moved from has move operations that are no faster than its copy operations.

+ **Move not usable**: The context in which the moving would take place requires a move operation that emits no exceptions, but that operation isn't declared **noexcept**.

+ **Source object is lvalue**: With very few exceptions, only rvalues may be used as the source of a move operation.

However, with all that being said, if you know the types you are working with or are sure that move operations are well-supported, you could just proceed as usual.