#pragma once

#include"../../implementation/objects_and_calls/function_evaluations.h"

namespace expr
{
    //unique_ptr wrap a bit more convenient and treats its memory as if it is part of the object(in terms of const-ness)
    //using impl::mu::virt;

    //virtual base class for something that can perform a call on some objects
    using impl::any_callable;

    //wraps any_callable
    using impl::held_callable;

    //derives from any_callable, invokes with a std::vector<stack_elem> without helping out much
    using impl::manual_callable;

    //derives from any_callable, invokes with any given types if correct ones are passed.
    using impl::smart_callable;

    //gets the value of a stack_elem as an object_holder (given that it actually has an object)
	using impl::get_value;

	//sets the object holder held by a stack_elem (given that it actually has an object)
	using impl::set_value;

	//a virtual base class of object_of, has methods for generic access.
	using impl::any_object;

	//can be of any type, derives from any_object
	using impl::object_of;

	//creates an object_of
	using impl::make_object;

	//used instead of object_of<void>
	using impl::void_object;

	//wraps any_object
	using impl::object_holder;

	//a reference to an object/variable
	using impl::variable_reference;

	//like an any_object but can also be a reference to an object/variable
	using impl::value_elem_val;

	//like a value_elem_val, but it can also hold a literal
	using impl::any_elem_val;

	//wraps any_elem_val
	using impl::stack_elem;

	//parsing/printing conversions generic to any type t
	//specialize this class to supply custom parsing and printing for a type
	using impl::type_operation_info;

	//a map of names to types
	using impl::type_info_set;

	//checks the name of a type in a type_info_set, by default using the global type dataset
	using impl::name_of;

	//a global type dataset
	using impl::global_type_info;

	//pre_call_t<t> is the object stored before being passed into a function argument of t
	using impl::pre_call_t;

	//post_return_t<t> is the object stored after a function returns a t
	using impl::post_return_t;

	//allows for manual parsing of newlines
	using impl::raw_istream_iter;

}
