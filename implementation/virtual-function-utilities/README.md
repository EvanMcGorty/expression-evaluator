# virtual-function-utilities
a bunch of random virtual function utilities

free-store_virtual.h has a class "virt", which is a wrap on unique pointer that can't be null unless you move out of it. It has upcasting and downcasting.

stack-based_virtual.h has a class "stack_virt", which is a bunch of space (stored on the stack) on which it can dynamically construct types and let you access their virtual methods.

algebraic_virtual.h has a class "algebraic", which is supposed to satisfy the need for algebraic data types in c++. It is a wrap on stack_virt that can only be one of a set amount of derived classes.
