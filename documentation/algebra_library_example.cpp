#include<cmath>
#include<vector>


class polynomial
{
public:

	void simplify()
	{
		while (coefficients.size() > 0 && *coefficients.rbegin() == 0)
		{
			coefficients.pop_back();
		}
	}

	polynomial() = default;
	polynomial(polynomial const&) = default;
	polynomial(polynomial&&) = default;
	polynomial& operator=(polynomial const&) = default;
	polynomial& operator=(polynomial&&) = default;

	polynomial(std::vector<double>&& a)
	{
		coefficients = std::move(a);
		simplify();
	}

	polynomial operator+(polynomial const& rhs) const
	{
		polynomial ret{};
		auto li = coefficients.begin();
		auto ri = rhs.coefficients.begin();

		while (li != coefficients.end() || ri != rhs.coefficients.end())
		{
			ret.coefficients.push_back(0);
			if (li != coefficients.end())
			{
				*ret.coefficients.rbegin() += *li;
				++li;
			}
			if (ri != rhs.coefficients.end())
			{
				*ret.coefficients.rbegin() += *ri;
				++ri;
			}
		}

		ret.simplify();

		return ret;
	}

	polynomial operator*(polynomial const& rhs) const
	{
		polynomial ret{};
		if (coefficients.size() == 0 || rhs.coefficients.size() == 0)
		{
			return ret;
		}
		for (int i = 0; i != (coefficients.size() + rhs.coefficients.size() - 1); ++i)
		{
			ret.coefficients.push_back(0);
		}
		for (int li = 0; li != coefficients.size(); ++li)
		{
			for (int ri = 0; ri != rhs.coefficients.size(); ++ri)
			{
				ret.coefficients[li + ri] += coefficients[li] * rhs.coefficients[ri];
			}
		}

		ret.simplify();

		return ret;
	}


	double substitute(double x) const
	{
		double ret = 0;
		for (int i = 0; i != coefficients.size(); ++i)
		{
			ret += coefficients[i] * std::pow(x, i);
		}
		return ret;
	}

	std::vector<double> coefficients;
};


#include<functional>
#include<iostream>

#include"../include/expression-evaluator/evaluator.h"

using namespace expr;

int main()
{
	//(using the global database) assign the polynomial class the name "poly", and similar for other classes.
	declare_with_name<polynomial>("poly");
	declare_with_name<double>("num");
	//vector is a type that the evaluator can automatically name. currently it will default to "num-vector" (in this case).
	//note that if this had been called first, it would default to something like "double-vector" or "float64-vector"
	declare<std::vector<double>>();


	//create the environment/settings of an interactive interpreter with default options
	interpreter e;
	//make the result of a full expression always passed to _view
	e.settings.auto_call("_view");

	//import the function set of polynomial utilities, which are only generic class semantics, and terminal utilities (through the namespace "console")
	e.functions.use<util<polynomial>>().use<terminal>("console");

	//then, using an alternate syntax, import (without a namespace) the core functions
	e.functions << "" << fs_functs<core>() << fs_functs<cpp_core>()

		//and also the vector utilities, which comes with some extra functions
		<< "vec" << fs_functs<util<std::vector<double>>>()

		//for the function str_to_obj, which can be used to load an object from one runtime to the next
		<< "" << fs_functs<types>()

		//then, with the name "poly", add a constructor function from vector<double>&& into polynomial
		<< "poly" << cfn<polynomial, std::vector<double>&&>()

		//then add some utilities for double math as smart functions and as values
		<< "sum" << sfn(std::function<double(double, double)>{[](double a, double b) {return a + b; }})
		<< "prod" << sfn(std::function<double(double, double)>{[](double a, double b) {return a * b; }})
		<< "num" << sfn(&util<double>::basic::copy_construct)
		<< "pi" << copier(3.1415926535)
		<< "e" << copier(2.7182818284)
		<< "num" << fs_functs<util<double>>()

		//then add overloads for sum and prod that work with polynomials (note that these overloads have less priority)
		<< "sum" << sfn(&polynomial::operator+) << "prod" << sfn(&polynomial::operator*)

		//then, as smart functions, add the various utility functions from the polynomial class
		<< "subst" << sfn(&polynomial::substitute) << "simp" << sfn(&polynomial::simplify)

		//and, finally, add member access to polynomial's coefficients
		<< "view" << mbr(&polynomial::coefficients);



	std::cout << "welcome to my algebra library!\n" << std::flush;

	//start the interpreter
	e.go();
	/*
	try entering:
_funcs
swap(=v/,vec.make(2,3))
vec.append(=v,clone(=v))
=v
swap(=p/,poly(=v))
subst(=p,3)
subst(=p,7)
swap(=p,prod(=p,poly([3 2 1 0])))
swap(=n/,vec.at(view(=p),3))
num.give(=n,prod(take(=n),take(=n)))
view(=p)
swap(subst(sum(=p,poly([0 0 0 0 0 0 0 0 0 1])),prod(prod(sum(1,2),sum(3,4)),sum(e,pi))),=p/)
_vars
_build_vars('str_to_obj)
_exit

	you can copy the result of _build_vars next time to load the same variables.
	however, not all types can be loaded. references wont work for obvious reasons and
	polynomial is a custom type that is this example does not provide parsing functions for.
	so, some values might not transfer over.
	*/

	std::cout << "have a good day!\n" << std::flush;
	std::string s;
	std::getline(std::cin, s);
	return 0;
}