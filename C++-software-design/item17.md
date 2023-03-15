### Guideline17: Consider std::variant for Implementing Visitor
+ Understand the architectural similarity between the classic Visitor and std::variant.
+ Be aware of the advantage of std::variant in comparison to an object-oriented Visitor solution.
+ Use the nointrusive nature of std::variant to create abstractions on the fly.
+ Keep in mind the shortcomings of std::variant and avoid it when it's not appropriate.