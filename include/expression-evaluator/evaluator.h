#pragma once

#include"../../implementation/smart_environments.h"


namespace expr
{

	//creates a callable object that will return a copy of the value passed into copier
	using impl::copier;

	//creates a callable object that will return a reference to the object into refto
	using impl::refto;

	//creates a callable object that will return a reference to the object into refto
	using impl::constrefto;

	//creates a callable object that provides member access based on the member pointer passed in
	using impl::mbr;

	//creates a callable object from a function of object_holder(vector<stack_elem>)
	//gives the function the raw data that was passed to deal with manually
	using impl::mfn;

	//creates a callable object from any function type with smart auto-conversions
	using impl::sfn;

	//creates a callable object that static casts from the second template parameter into the first.
	using impl::cfn;

	//gets a function set from a import class as a template argument (for example fs_functs<core>())
	using impl::fs_functs;

	//utility for picking a specific instanciation of a function with the type of its template parameter.
	using impl::pick;

	//organizes variables, functions, and a garbage stack into an environment that can run code.
	using impl::environment;

	//derives from environment, provides a python-like command line utility.
	using impl::interpreter;

	//an organization of callable objects
	using impl::function_set;

	//renames a type within a typename dataset (by default uses a global one if you include features/global_type_info_object.h)
	using impl::declare_with_name;

	//does declare_with_name on arbitrarily many types using their default names
	using impl::declare;

	//core utilities to the evaluator
	using impl::core;

	//core utilities specific to c++
	using impl::cpp_core;

	//utilities generic to any type
	using impl::util;

}