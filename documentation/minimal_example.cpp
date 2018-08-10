/*
this is a simple, minimal example use of the evaluator

compile this with:

g++ minimal_example.cpp -std=c++17
or
clang-cl minimal_example.cpp /GX /std:c++17
or
clang++ minimal_example.cpp -std=c++17 -Xclang -flto-visibility-public-std

on windows g++ might need -Os to optimize for size

for msvc, cpp files may need to be set with "bigobj" in their properties.

requires c++17 support, runtime type info, and exceptions.

when the terminal opens, try entering the following lines:
_funcs
prod(2,3)
push(8.4)
=my_double/
swap(=my_double,double.make(3.14))
=my_double
push(prod(=my_double,-1/2))
rotate(=my_double)
=my_double
print(glv)
swap(=my_vec/,vec.copy-make(glv))
vec.append(=my_vec,vec.list-make(12345))
print(=my_vec)
vec.swap(glv,=my_vec)
print(=my_vec)
rotate(=my_double)
vec.swap(glv,=my_vec)
print(=my_vec)
=my_vec\
_garb(=g/)
=g
drop(=g)
=g
_garb(=g)
=g
=my_double\
_garb(=g)
=g
strong(double.make(4))
_garb(=g)
=g
drop(=g\)
_exit

*/

#include <vector>
#include <iostream>

#include"../include/evaluator.h"

using namespace expr;

double multiply(double a, double b)
{
	return a * b;
}

std::vector<double> global_vector;

void push(double a)
{
	global_vector.push_back(a);
}

void rotate(double &a)
{
	for (double &i : global_vector)
	{
		std::swap(a, i);
	}
}

void print(std::vector<double> const &w)
{
	for (auto const &i : w)
	{
		std::cout << i << '\n' << std::flush;
	}
}

int main()
{
	std::cout << "this code will run before the evaluator is used\n" << std::flush;

	//by default uses a global rename dataset
	declare_with_name<double>("num");
	declare<std::vector<double>>();

	//the environment that will hold variables and functions
	environment env;

	//use imports a function set. an empty string means to not use a namespace
	env.functions.use<core>("").use<cpp_core>("")

		//util<t> wraps t and provides basic functions. by not providing a string, the evaluator chooses a default name.
		.use<util<std::vector<double>>>()
		.use<util<double>>()

		//a pretty syntax for binding functions. sfn takes a function pointer and turns it into something that env can use.
		<< "prod" << sfn(multiply)
		<< "push" << sfn(push)
		<< "rotate" << sfn(rotate)
		<< "print" << sfn(print)

		//val creates a function that returns a copy of the value it is passed. passing a pointer to global_vector gives write access to the caller.
		<< "glv" << refto(global_vector);

	//there are alternate ways to expresss binding functions.
	//function sets can also be imported with the << syntax using fs_functs
	//and functions can be bound with method syntax with .add

	//interpreter is a more complicated environment that interacts with iostreams and has settings (with default settings and cout/cin).
	interpreter{ std::move(env) }.go();

	std::cout << "this code will run after the user calls _exit from the interpreter\n" << std::flush;
	char a;
	std::cin >> a;
}