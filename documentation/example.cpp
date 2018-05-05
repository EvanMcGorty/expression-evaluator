/*
this is a simple example use of the evaluator

compile this with:
g++ example.cpp -std=c++17
or
clang-cl example.cpp /std:c++17

requires c++17 support and runtime type info

when the terminal opens, try entering the following lines:
functs
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
push(=my_vec,12345)
print(=my_vec)
vec.swap(glv,=my_vec)
print(=my_vec)
rotate(=my_double)
vec.swap(glv,=my_vec)
print(=my_vec)
=my_vec\
gar
drop(gar)
gar
=my_double\
gar
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
    expr::rename<std::vector<double>>("vec");

    expr::environment env;

    env .fbind("prod",&multiply)
        .fbind("push",&push)
        .fbind("rotate",&rotate)
        .fbind("size",&std::vector<double>::size)
        .fbind("push",&std::vector<double>::emplace_back<double&&>)
        .fbind("print",&print)

        .vbind("glv",&global_vector)

        .sbind<expr::core>("")
        .sbind<expr::cpp_core>("")
        .sbind<expr::util<std::vector<double>>>()
        .sbind<expr::util<double>>()

        .rbind("gar",env.garbage_getter())
        .rbind("view",env.info_printer(std::cout))
        .rbind("vars",env.variables_printer(std::cout))
        .rbind("functs",env.functions_printer(std::cout));

	expr::option_set options;
	options.auto_call("view");

	env.attach(std::cin, std::cout, std::move(options));
}