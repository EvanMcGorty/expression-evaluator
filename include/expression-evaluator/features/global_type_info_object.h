#pragma once
#include"../../../implementation/smart_environments.h"


namespace expr
{
	namespace impl
	{


		static type_info_set* global_type_info_object_pointer;

		inline void delete_global_type_info_object()
		{
			delete global_type_info_object_pointer;
		}


		static type_info_set*& make_global_type_info_object()
		{
			std::atexit(&delete_global_type_info_object);
			global_type_info_object_pointer = new type_info_set();
			return global_type_info_object_pointer;
		}

		type_info_set* global_type_info()
		{
			static type_info_set*& val = make_global_type_info_object();
			return val;
		}

		template <typename t>
		void declare_with_name(std::string&& new_name)
		{
			declare_with_name<t>(std::move(new_name), global_type_info());
		}

		template <typename t>
		void declare()
		{
			declare<t>(global_type_info());
		}

		template <typename t>
		std::string name_of()
		{
			return name_of<t>(global_type_info());
		}

		template<typename t, typename string_convertible>
		function_set& function_set::use(function_set&& set)
		{
			return use(fs_name<t>(global_type_info()),std::move(set));
		}

		function_set fs_info<types>::get_functions()
		{
			return fs_info<types>::get_functions(global_type_info());
		}


		stack environment::run(executable&& a, std::ostream& errors)
		{
			return run(std::move(a), errors, global_type_info());
		}


		stack_elem environment::evaluate(expression&& a, std::ostream& errors)
		{
			return evaluate(std::move(a), errors, global_type_info());
		}

		held_callable environment::info_printer(std::ostream& to)
		{
			return info_printer(global_type_info(), to);
		}

		held_callable environment::functions_printer(std::ostream& to)
		{
			return functions_printer(global_type_info(), to);
		}

		held_callable environment::variables_printer(std::ostream& to)
		{
			return variables_printer(global_type_info(), to);
		}

		held_callable environment::variables_builder(std::ostream& to)
		{
			return variables_builder(global_type_info(), to);
		}

		interpreter::interpreter(environment&& base, std::istream& i, std::ostream& o, option_set&& s) :
			interpreter(global_type_info(), std::move(base), i, o, std::move(s))
		{}

	}

	using impl::global_type_info;

}