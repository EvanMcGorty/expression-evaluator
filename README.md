# expression-evaluator
A library that lets you bind string names to your functions and parse expressions that try to call these functions at runtime.

This library takes advantage of template meta-programming, variadic generics, and polymorphic types to make it incredibly easy to bind functions with names, and store arbitrary types in a powerful variable system.
It can also be used simply for creating and manipulating expression trees for algebra or lambda calculus, without even taking advantage of the expression evaluation.

# philosophy
the expression evaluator system has a fairly simple philosophy that focuses on minimalism and "genericness".

the expression evaluator has nothing to do with c++. it could be implemented to interact with practically any programming language. there are functions and objects. objects hold bits, and functions do stuff with those bits, and might also return bits. a function might just read the bits, it might write to them, or it might take the bits for itself.
variables can hold an object with as many bits as they want. You can give their objects to functions, to read, write to, or take.
literals are meaningless strings. any implementation of an expression evaluator should decide how literals should be translated into bytes.
the expression evaluator is allowed to keep track of your bits (where you got them from, what you've dont to them, where you're putting them), and tell you if you are doing something dangerous, but it isn't allowed to edit your bits if you dont say so.

# c++
this "c++ implementation" for the expression evaluator prioritizes on making it convenient to quickly bind existing code, while strongly enforcing the c++ type system. Because c++ uses the concept of "resources" (classes have constructors and destructors), controling the consumption of resources/objects is difficult and confusing. Different function signatures do different things to the objects that they are passed, and there is a distinction between references and pointers.

for a function "f" taking an argument of type "t", the function signatures f(t x) and f(const t x) only need to read from the object, at least given that x has a copy constructor, the function signatures f(const t& x) and f(const t&& x) also only need to read from the object, the function signature f(t& x) needs to have both read and write access to the object, and the function signature f(t&& x) needs to take x all for itself to consume its resources.