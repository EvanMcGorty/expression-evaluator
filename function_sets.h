#pragma once
#include"statement_evaluations.h"

namespace expr
{
	namespace impl
	{
		
		template<typename t>
		class fs_info
		{
		public:
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
		std::string fs_name()
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
					return value_holder::make<void_object>(); //indicates a successful function call even though the return type is void
				}
				else
				{
					return value_holder::make_nullval();
				}
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

		};

		template<>
		struct fs_info<core>
		{
			static function_set get_functions()
			{
				function_set ret;
				ret.add(manual(core::swap), "swap")
					.add(manual(core::first), "first")
					.add(manual(core::last), "last");
				return ret;
			}

			static std::string get_name()
			{
				return "core";
			}
		};

		struct cpp_core
		{

			static value_holder to_string(std::vector<stack_elem>& a)
			{
				if (a.size() == 1 && !a[0].is_nullval())
				{
					return value_holder::make<object_of<std::string>>(a[0]->convert_into_string());
				}
				else
				{
					return value_holder::make_nullval();
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
				return value_holder::make<void_object>(); //indicates a successful function call even though the return type is void
			}
		};

		template<>
		struct fs_info<cpp_core>
		{
			static function_set get_functions()
			{
				function_set ret;
				ret.add(manual(cpp_core::drop), "drop")
					.add(manual(cpp_core::to_string), "to_string");
				return ret;
			}

			static std::string get_name()
			{
				return "cpp";
			}
		};

		template<typename t>
		struct util
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

			static void swap(t& a, t& b)
			{
				std::swap(a, b);
			}

			static void destruct(t&& a)
			{
				//let evaluator call destructor naturally
			}

			static t&& temporary_reference(t& a)
			{
				return std::move(a);
			}

			static t& mutable_reference(t& a)
			{
				return a;
			}

			static t const& const_reference(t& a)
			{
				return a;
			}

		};

		template<typename t>
		struct fs_info<util<t>>
		{
			static function_set get_functions()
			{
				function_set ret;
				if constexpr(std::is_default_constructible<t>::value)
				{
					ret.add(callable(as_function(&util<t>::default_construct)), "make");
				}
				if constexpr(std::is_move_constructible<t>::value)
				{
					ret.add(callable(as_function(&util<t>::move_construct)), "make");
					ret.add(callable(as_function(&util<t>::move_construct)), "move-make");
				}
				if constexpr(std::is_copy_constructible<t>::value)
				{
					ret.add(callable(as_function(&util<t>::copy_construct)), "make");
					ret.add(callable(as_function(&util<t>::copy_construct)), "copy-make");
				}
				if constexpr(std::is_move_assignable<t>::value)
				{
					ret.add(callable(as_function(&util<t>::move_assign)), "give");
					ret.add(callable(as_function(&util<t>::move_assign)), "move-give");
				}
				if constexpr(std::is_copy_assignable<t>::value)
				{
					ret.add(callable(as_function(&util<t>::copy_assign)), "give");
					ret.add(callable(as_function(&util<t>::copy_assign)), "copy-give");
				}
				ret.add(callable(as_function(&util<t>::swap)), "swap");
				ret.add(callable(as_function(&util<t>::destruct)),"drop");
				ret.add(callable(as_function(&util<t>::temporary_reference)), "tref");
				ret.add(callable(as_function(&util<t>::mutable_reference)), "mref");
				ret.add(callable(as_function(&util<t>::const_reference)), "cref");
				return ret;
			}

			static std::string get_name()
			{
				return name_of<t>();
			}
		};

	}

}