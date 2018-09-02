#pragma once

#include"evaluator.h"

namespace expr
{
	//a set of options for an interpreter
	using impl::option_set;

	//an expression tree put into the form of a stack that can be evaluated
	using impl::executable;

	//a variant type that holds wrapped functions that can be called with virtual methods
	using impl::held_callable;

	//a class that wraps an import class to access the name of the import and it's functions
	using impl::fs_info;

	//uses fs_info to get the name of an import class
	using impl::fs_name;

	//for supplying information and methods for types
	using impl::type_operation_info;
	using impl::wrapper_type_operation_info;

	using impl::default_type_info;

}
