#pragma once
#include"type_demangle.h"
#include"statement_evaluations.h"

namespace expr
{
	namespace impl
	{
		template<typename t>
		function_set get_all_functions()
		{
			static_assert("no overloaded instance of get_all_functions exists for this class");
		}

		template<typename t>
		std::string get_name()
		{
			static_assert("no overloaded instance of get_name exists for this class");
		}

		struct core
		{

			static value_holder swap(std::vector<stack_elem>& a)
			{
				if (a.size() == 2 && (a[0].is_nullval() || a[0]->has_value()) && (a[0].is_nullval() || a[1]->has_value()))
				{
					auto o0 = get_value(a[0]);
					auto o1 = get_value(a[1]);
					set_value(a[0], std::move(o1));
					set_value(a[1], std::move(o0));
				}
				return value_holder::make_nullval();
			}

			static value_holder first(std::vector<stack_elem>& a)
			{
				if (a.size() == 0 || (*a.begin()).is_nullval() || !(*a.begin())->has_value())
				{
					return value_holder::make_nullval();
				}
				else
				{
					return get_value(*a.begin());
				}
			}


			static value_holder last(std::vector<stack_elem>& a)
			{
				if (a.size() == 0 || (*a.rbegin()).is_nullval() || !(*a.rbegin())->has_value())
				{
					return value_holder::make_nullval();
				}
				else
				{
					return get_value(*a.rbegin());
				}
			}

			static value_holder drop(std::vector<stack_elem>& a)
			{
				for (auto& it : a)
				{
					if (!it.is_nullval())
					{
						if (it->has_value())
						{
							get_value(it);
						}
						else
						{
							continue;
						}
					}
				}
				return value_holder::make_nullval();
			}
		};

		template<>
		function_set get_all_functions<core>()
		{
			function_set ret;
			ret.add(make_manual_callable(core::swap), "swap")
				.add(make_manual_callable(core::first), "first")
				.add(make_manual_callable(core::last), "last")
				.add(make_manual_callable(core::drop), "drop");
			return ret;
		}
		template<>
		std::string get_name<core>()
		{
			return "core";
		}

		template<typename t>
		struct basics
		{
			t default_construct()
			{
				if constexpr(std::is_default_constructible<t>::value)
				{
					return t{};
				}
				else
				{
					assert(false);
				}
			}

			t copy_construct(t const& a)
			{
				return std::move(a);
			}

			t move_construct(t&& a)
			{
				return std::move(a);
			}

			void move_assign(t& a, t&& b)
			{
				a = std::move(b);
			}

			void copy_assign(t& a, t const& b)
			{
				a = b;
			}

			void destruct(t&& a)
			{
				//destructor would run automatically when called by the evaluator
			}

		};

		template<typename t>
		template<>
		function_set get_all_functions<basics<t>>()
		{
			function_set ret;
			ret.add(make_smart_callable(basics<t>::construct), "make")
			.add(make_smart_callable(basics<t>::move_construct), "move_make")
			.add(make_smart_callable(basics<t>::copy_construct), "copy_make")
			.add(make_smart_callable(basics<t>::move_assign), "move")
			.add(make_smart_callable(basics<t>::copy_assign), "copy")
			.add(make_smart_callable(basics<t>::destruct), "drop")
			return ret;
		}


		template<typename t>
		template<>
		std::string get_name<basics<t>>()
		{
			return demangle(typeid(t).name());
		}

	}

}