### Guideline23: Prefer a Value-Based Implementation of Strategy and Command
+ Consider using std::function to implement the Strategy or Command design pattern.
+ Take the performance disadvantage of std::function into account.
+ Be aware that Type Erasure is a generalization of the value semantics approach for Strategy and Command.