# expression-evaluator
A library that lets you bind string names to your functions and parse expressions that try to call these functions at runtime.

This library takes advantage of template meta-programming, variadic generics, and polymorphic types to make it incredibly easy to bind functions with names and store arbitrary types in a powerful variable system.
It can also be used simply for creating and manipulating expression trees for algebra or lambda calculus, without even taking advantage of the expression evaluation.

## philosophy
the expression evaluator system has a fairly simple philosophy that focuses on minimalism and "genericness".

the expression evaluator has nothing to do with c++ and could be implemented to interact with practically any programming language. the general idea is that there are functions and objects. objects hold bits, and functions do stuff with those bits, and might also return bits. a function might just read the bits, it might write to them, or it might take the bits for itself.
variables can hold an object with as many bits as they want. You can give their objects to functions, to view, edit, or take.
literals are meaningless strings, and each implementation of the expression evaluator should decide how literals should be translated into bytes depending on the context.
the expression evaluator is allowed to keep track of your bits (where you got them from, what you've done to them, where you're putting them), and stop you if you are doing something dangerous, but it isn't allowed to edit your bits if you don't say so.
an implementation of the evaluator is only allowed to call extra code implicitly if doing so is part of its definition for calling functions.

## c++
this "c++ implementation" for the expression evaluator prioritizes on making it convenient to quickly bind existing code, while strongly enforcing the c++ type system. because c++ enforces the concept of "resources" (classes have constructors and destructors), controlling the consumption of resources/objects is difficult. Different function signatures do different things to the objects that they are passed, and there is a confusing distinction between references and pointers.

for a function "f" taking an argument of type "t", the function signatures f(const t& x) and f(const t&& x) only need to read from the object, the function signature f(t& x) needs to have both read and write access to the object, and the function signatures f(t x), f(t const x), and f(t&& x) need a whole t for themselves to consume its resources.

a c++ function doesn't always consume the "bits" that it's given. non-const rvalue references and raw values are the only kinds of signatures for which the evaluator will steal resources (with a std::move) and let the destructor run (on the value that std::move was called on). When a non-consuming argument is passed an object that isn't owned by a variable, the function couldn't know that it needed to call some destructor on those bits, and automatically running a destructor goes against the evaluator's philosophy. to avoid resource leaks, a garbage-object stack is kept for types with non-trivial destructors and can be exposed to a user if necessary.

a few other notes:
- objects are kept virtually on the free-store, so move constructors don't need to be called implicitly.
- when they are passed for a non-const rvalue reference, objects that are trivially copy constructible and trivially move constructible will not be consumed.
- lvalue references and pointers are interchangable.
- an object of type "t" can be passed for a t*, t&, t const*, or t const&. 
- an object of type "t*"/"t&"/"unique_ptr<<t>t>" can be passed for a t const*, t const&, t*, t&, t, or a remove_const_t<<t>t>
- (currently) passed objects will not be implicitly upcast or copy/move converted to the type in the function signature.
- as a workaround, there is a simple overload resolution system that can be used for type conversions.
- if a function cannot run with the types it has been passed, it will still free resources that it has been passed.

## using the evaluator library
the project wiki page on GitHub will contain all documentation
