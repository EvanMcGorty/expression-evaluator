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

		while (li!=coefficients.end() && ri!=rhs.coefficients.end())
		{
			ret.coefficients.push_back(0);
			if (li != coefficients.end())
			{
				*ret.coefficients.rbegin() += *li;
				++li;
			}
			if (ri == rhs.coefficients.end())
			{
				*ret.coefficients.rbegin() += *ri;
				++li;
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
		for (int i = 0; i != (coefficients.size()+rhs.coefficients.size()-1); ++i)
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
		for (int i = 0; i!=coefficients.size(); ++i)
		{
			ret += coefficients[i] * std::pow(x, i + 1);
		}
		return ret;
	}


	std::vector<double>& data()
	{
		return coefficients;
	}



private:
	std::vector<double> coefficients;
};

#include<functional>

#include"../evaluator.h"

using namespace expr;

int main()
{
	//(using the global database) assign the polynomial class the name "poly", and similar for other classes
	rename<polynomial>("poly"); 
	rename<double>("num");
	rename<std::vector<double>>("vec");


	//create the environment/settings of an interactive interpreter with default options
	interpreter e;

	//import the function set of polynomial utilities, which are only generic class semantics, and terminal utilities (through the namespace "console")
	e.functions.use<util<polynomial>>().use<terminal>("console");

	//then, using an alternate syntax, import (without a namespace) the core functions
	e.functions << "" << fs_functs<core>() << fs_functs<cpp_core>()

		//and also the vector utilities, which comes with some extra functions
		<< "vec" << fs_functs<util<std::vector<double>>>()

		//then, with the name "poly.vec-make", add a constructor function from vector<double>&& into polynomial
		<< "poly.vec-make" << cfn<polynomial, std::vector<double>&&>()

		//then add some utilities for double math as smart functions and as values
		<< "sum" << sfn(std::function<double(double, double)>{[](double a, double b) {return a + b; }})
		<< "prod" << sfn(std::function<double(double, double)>{[](double a, double b) {return a * b; }})
		<< "num" << sfn(&util<double>::basic::copy_construct)
		<< "pi" << val(3.1415926535)
		<< "e" << val(2.7182818284)
		<< "num" << fs_functs<util<double>>()

		//then add overloads for sum and prod that work with polynomials (note that these overloads have less priority)
		<< "sum" << sfn(&polynomial::operator+) << "prod" << sfn(&polynomial::operator*)

		//then, as smart functions, add the various utility functions from the polynomial class
		<< "subst" << sfn(&polynomial::substitute) << "get-data" << sfn(&polynomial::data) << "simp" << sfn(&polynomial::simplify);

	

	std::cout << "welcome to my algebra library!\n" << std::flush;

	//start the interpreter
	e.go();
/*
try entering:
_funcs
swap(=v/,vec.make(2,3))
vec.append(=v,clone(=v))
=v
swap(=p/,poly.vec-make(=v))
subst(=p,3)
subst(=p,7)
swap(=p,prod(=p,poly.vec-make("[3,2,1,0]")))
swap(=n/,vec.at(get-data(=p),3))
num.give(=n,prod(take(=n),take(=n)))
get-data(=p)
subst(sum(=p,poly.vec-make("[0 0 0 0 0 0 0 0 0 1]")),prod(prod(sum(1,2),sum(3,4)),sum(e,pi)))
_vars
_exit

*/

	std::cout << "have a good day!\n" << std::flush;
	std::string s;
	std::getline(std::cin, s);
	return 0;
}