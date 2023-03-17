### Guideline34: Be Aware of the Setup Costs of Owning Type Erasure Wrappers
+ Keep in mind that the setup of owning Type Erasure wrappers many involve copy operations and allocations.
+ Be aware of nonowning Type Erasure, but also understand its reference semantics deficiencies.
+ Prefer Simple Type Erasure implementations, but know their limits.
+ Prefer to use nonowning Type Erasure for function arguments but not for data members or return types.