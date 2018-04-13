#pragma once
#include"type_demangle.h"
#include"statement_evaluations.h"

namespace expr
{
	namespace impl
	{
		
		template<typename t>
		class fs_info
		{
			friend function_set fs_functs();
			friend function_set fs_name();

			static function_set get_functions()
			{
				static_assert("no overloaded instance of fs_info exists for this class");
			}

			static std::string get_name()
			{
				static_assert("no overloaded instance of fs_info exists for this class");
			}
		};

		template<typename t>
		function_set fs_functs()
		{
			return fs_info<t>::get_functions();
		}
		template<typename t>
		function_set fs_name()
		{
			return fs_info<t>::get_name();
		}

		struct core
		{

			static value_holder swap(std::vector<stack_elem>& a)
			{
				if (a.size() == 2 && (a[0].is_nullval() || a[0]->has_value()) && (a[1].is_nullval() || a[1]->has_value()))
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
		struct fs_info<core>
		{
			static function_set get_functions()
			{
				function_set ret;
				ret.add(manual(core::swap), "swap")
					.add(manual(core::first), "first")
					.add(manual(core::last), "last")
					.add(manual(core::drop), "drop");
				return ret;
			}

			static std::string get_name()
			{
				return "core";
			}
		};

		template<typename t>
		struct basics
		{
			static t default_construct()
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

			static t copy_construct(t const& a)
			{
				if constexpr(std::is_copy_constructible<t>::value)
				{
					return a;
				}
				else
				{
					assert(false);
				}
			}

			static t move_construct(t&& a)
			{
				if constexpr(std::is_move_constructible<t>::value)
				{
					return std::move(a);
				}
				else
				{
					assert(false);
				}
			}

			static void move_assign(t& a, t&& b)
			{
				if constexpr(std::is_move_assignable<t>::value)
				{
					a = std::move(b);
				}
				else
				{
					assert(false);
				}
			}

			static void copy_assign(t& a, t const& b)
			{
				if constexpr(std::is_copy_assignable<t>::value)
				{
					a = b;
				}
				else
				{
					assert(false);
				}
			}

			static void destruct(t&& a)
			{
				//destructor would run automatically when called by the evaluator
			}

		};

		template<typename t>
		struct fs_info<basics<t>>
		{
			static function_set get_functions()
			{
				function_set ret;
				if constexpr(std::is_default_constructible<t>::value)
				{
					ret.add(callable(&basics<t>::default_construct), "make");
				}
				if constexpr(std::is_move_constructible<t>::value)
				{
					ret.add(callable(&basics<t>::move_construct), "move_make");
				}
				if constexpr(std::is_copy_constructible<t>::value)
				{
					ret.add(callable(&basics<t>::copy_construct), "copy_make");
				}
				if constexpr(std::is_move_assignable<t>::value)
				{
					ret.add(callable(&basics<t>::move_assign), "move");
				}
				if constexpr(std::is_copy_assignable<t>::value)
				{
					ret.add(callable(&basics<t>::copy_assign), "copy");
				}
				ret.add(callable(&basics<t>::destruct), "drop");
				return ret;
			}

			static std::string get_name()
			{
				return demangle(typeid(t).name());
			}
		};

	}

}