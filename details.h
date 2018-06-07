#include"evaluator.h"
#include"expression.h"

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

	//gets the value of a stack_elem as an object_holder (given that it actually has an object)
	using impl::get_value;

	//sets the object holder held by a stack_elem (given that it actually has an object)
	using impl::set_value;

	//a virtual base class of object_of, has methods for generic access.
	using impl::any_object;

	//can be of any type, derives from any_object
	using impl::object_of;

	//used instead of object_of<void>
	using impl::void_object;

	//wraps any_object
	using impl::object_holder;

	//a reference to an object/variable
	using impl::value_reference;

	//like an any_object but can also be a reference to an object/variable
	using impl::value_elem_val;

	//like a value_elem_val, but it can also hold a literal
	using impl::any_elem_val;

	//wraps any_elem_val
	using impl::stack_elem;

	//parsing/printing conversions generic to any type t
	//specialize this class to supply custom parsing and printing for a type
	using impl::converter;

	//a map of names to types
	using impl::name_set;

	//checks the name of a type in a name_set, by default using the global type dataset
	using impl::name_of;

	//a global type dataset
	using impl::global_type_renames;



}
