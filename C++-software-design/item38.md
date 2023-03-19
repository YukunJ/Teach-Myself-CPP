### Guideline38: Design Singletons for Change and Testability
+ Be aware that Singleton represents global state, with all its flaws.
+ Avoid global state as much as possible.
+ Use Singleton judiciously and just for the few global aspects in your code.
+ Prefer Singletons with unidirectional data flow.
+ Use the Strategy design pattern to invert dependencies on your Singleton to remove the usual impediments to changeability and testability.