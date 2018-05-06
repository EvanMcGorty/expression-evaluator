#pragma once
#include"environment_sets.h"

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

			static std::string get_name(name_set const& names)
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
		std::string fs_name(name_set const& names)
		{
			return fs_info<t>::get_name(names);
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

			static std::string get_name(name_set const& names)
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

			static std::string get_name(name_set const& names)
			{
				return "cpp";
			}
		};

		template<typename t>
		struct basic_util
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
		struct fs_info<basic_util<t>>
		{
			static function_set get_functions()
			{
				function_set ret;
				if constexpr(std::is_default_constructible<t>::value)
				{
					ret.add(callable(as_function(&basic_util<t>::default_construct)), "make");
				}
				if constexpr(std::is_move_constructible<t>::value)
				{
					ret.add(callable(as_function(&basic_util<t>::move_construct)), "make");
					ret.add(callable(as_function(&basic_util<t>::move_construct)), "move-make");
				}
				if constexpr(std::is_copy_constructible<t>::value)
				{
					ret.add(callable(as_function(&basic_util<t>::copy_construct)), "make");
					ret.add(callable(as_function(&basic_util<t>::copy_construct)), "copy-make");
				}
				if constexpr(std::is_move_assignable<t>::value)
				{
					ret.add(callable(as_function(&basic_util<t>::move_assign)), "give");
					ret.add(callable(as_function(&basic_util<t>::move_assign)), "move-give");
				}
				if constexpr(std::is_copy_assignable<t>::value)
				{
					ret.add(callable(as_function(&basic_util<t>::copy_assign)), "give");
					ret.add(callable(as_function(&basic_util<t>::copy_assign)), "copy-give");
				}
				ret.add(callable(as_function(&basic_util<t>::swap)), "swap");
				ret.add(callable(as_function(&basic_util<t>::destruct)),"drop");
				ret.add(callable(as_function(&basic_util<t>::temporary_reference)), "tref");
				ret.add(callable(as_function(&basic_util<t>::mutable_reference)), "mref");
				ret.add(callable(as_function(&basic_util<t>::const_reference)), "cref");
				return ret;
			}

			static std::string get_name(name_set const& from)
			{
				return name_of<t>(from);
			}
		};


		template<typename t>
		struct extended_util
		{};

		template<typename t>
		struct fs_info<extended_util<t>>
		{
			static function_set get_functions()
			{
				return function_set{};
			}

			static std::string get_name(name_set const& from)
			{
				return name_of<t>(from);
			}
		};


		template<typename t>
		struct util
		{
			typedef basic_util<t> basic;
			typedef extended_util<t> extended;
		};

		template<typename t>
		struct fs_info<util<t>>
		{
			static function_set get_functions()
			{
				function_set basic = fs_info<basic_util<t>>::get_functions();
				function_set extended = fs_info<extended_util<t>>::get_functions();
				return std::move(extended.merge(std::move(basic)));
			}

			static std::string get_name(name_set const& from)
			{
				return name_of<t>(from);
			}
		};


		template<>
		struct extended_util<std::string>
		{
			static std::string from_c_str(char const* from)
			{
				return std::string{ from };
			}

			static char const* as_c_str(std::string const& tar)
			{
				return tar.c_str();
			}

			static char& index(std::string& tar, size_t at)
			{
				return tar[at];
			}

			static char const& const_index(std::string const& tar, size_t at)
			{
				return tar[at];
			}

			static void append(std::string& a, std::string const& b)
			{
				a.append(b);
			}

			static void resize(std::string& a, size_t to)
			{
				a.resize(to);
			}

			static size_t size(std::string const& a)
			{
				return a.size();
			}

		};

		template<>
		struct fs_info<extended_util<std::string>>
		{
			static function_set get_functions()
			{
				function_set ret;
				ret
					.add(callable(as_function(&extended_util<std::string>::as_c_str)), "as-c_str")
					.add(callable(as_function(&extended_util<std::string>::from_c_str)), "c_str-make")
					.add(callable(as_function(&extended_util<std::string>::from_c_str)), "make")
					.add(callable(as_function(&extended_util<std::string>::index)), "at")
					.add(callable(as_function(&extended_util<std::string>::const_index)), "const-at")
					.add(callable(as_function(&extended_util<std::string>::append)), "append")
					.add(callable(as_function(&extended_util<std::string>::resize)), "resize")
					.add(callable(as_function(&extended_util<std::string>::size)), "len");
				return std::move(ret);
			}

			static std::string get_name(name_set const& from)
			{
				return name_of<std::string>(from);
			}
		};


	}

}