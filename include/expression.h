#pragma once

#include"../implementation/objects_and_calls/expressions/tree_structure.h"

namespace expr
{
	//a variant class representing an expression.
	//it can be a literal, a variable name, or a function with more elems
	using impl::expression;

	//the data type of a literal in an expression tree
	using impl::literal_value;

	//the data type of a variable in an expression tree
	using impl::variable_value;

	//the data type of a function in an expression tree
	using impl::function_value;

	//the enum type for variables indicating a pop, a push, or nothing.
	using impl::sc;
}
