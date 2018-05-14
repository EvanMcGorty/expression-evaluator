/*
this is a simple example use of the evaluator

compile this with:
g++ example.cpp -std=c++17
or
clang-cl example.cpp /GX /std:c++17
or
clang++ source.cpp -std=c++17 -Xclang -flto-visibility-public-std

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

#include<vector>
#include<iostream>

#include "../evaluator.h"


double multiply(double a, double b)
{
    return a*b;
}

std::vector<double> global_vector;


void push(double a)
{
    global_vector.push_back(a);
}

void rotate(double& a)
{
    for(double& i : global_vector)
    {
        std::swap(a,i);
    }
}

void print(std::vector<double> const& w)
{
    for(auto const& i : w)
    {
        std::cout << i << std::endl;
    }
}

int main()
{
    std::cout << "this code will run before the evaluator is used" << std::endl;

    expr::rename<std::vector<double>>("vec");

    expr::environment env;

    env .fbind("prod",&multiply)
        .fbind("push",&push)
        .fbind("rotate",&rotate)
        .fbind("print",&print)

        .vbind("glv",&global_vector)

        .sbind<expr::core>("")
        .sbind<expr::cpp_core>("")
        .sbind<expr::util<std::vector<double>>>()
        .sbind<expr::util<double>>();

	env.attach();
    
    std::cout << "this code will run after the user calls _exit from the evaluator" << std::endl;
    char a;
    std::cin >> a;
}