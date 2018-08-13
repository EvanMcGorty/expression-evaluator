#pragma once
#include<exception>
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
				return function_set{};
			}

			static std::string get_name(type_info_set const& names)
			{
				static_assert("no overloaded instance of fs_info exists for this class");
				return "";
			}
		};

		template<typename t, typename...fs_arg_ts>
		function_set fs_functs(fs_arg_ts&&...fs_args)
		{
			return fs_info<t>::get_functions(std::forward<fs_arg_ts>(fs_args)...);
		}
		template<typename t>
		std::string fs_name(type_info_set const& names)
		{
			return fs_info<t>::get_name(names);
		}

		template <typename t, typename string_convertible>
		function_set &function_set::use(string_convertible &&n, function_set &&set)
		{
			std::string name{n};
			if (name == "")
			{
				for (auto it = set.map.begin(); it != set.map.end(); ++it)
				{
					add_to_map(std::move(*it));
				}
				return *this;
			}
			assert_with_invalid_name_usage([&]() { return name_checker::is_valid(std::string{name}); });
			for (auto it = set.map.begin(); it != set.map.end(); ++it)
			{
				auto cur = std::move(*it);
				add_to_map(std::make_pair(std::string{name + "." + cur.first}, std::move(cur.second)));
			}
			return *this;
		}

		template<typename t>
		struct basic_util
		{
			static t default_construct()
			{
				if constexpr(std::is_default_constructible_v<t>)
				{
					return t{};
				}
				else
				{
					assert_with_invalid_method_usage([&]() {return false; });
				}
			}

			static t copy_construct(t const& a)
			{
				if constexpr(std::is_copy_constructible_v<t>)
				{
					return a;
				}
				else
				{
					assert_with_invalid_method_usage([&]() {return false; });
				}
			}

			static t move_construct(t&& a)
			{
				if constexpr(std::is_move_constructible_v<t>)
				{
					return std::move(a);
				}
				else
				{
					assert_with_invalid_method_usage([&]() {return false; });
				}
			}

			static void move_assign(t& a, t&& b)
			{
				if constexpr(std::is_move_assignable_v<t>)
				{
					a = std::move(b);
				}
				else
				{
					assert_with_invalid_method_usage([&]() {return false; });
				}
			}

			static void copy_assign(t& a, t const& b)
			{
				if constexpr(std::is_copy_assignable_v<t>)
				{
					a = b;
				}
				else
				{
					assert_with_invalid_method_usage([&]() {return false; });
				}
			}

			static void swap(t& a, t& b)
			{
				std::swap(a, b);
			}

		};

		template<typename t>
		struct fs_info<basic_util<t>>
		{
			static function_set get_functions()
			{
				function_set ret;
				if constexpr(std::is_default_constructible_v<t>)
				{
					ret.add(sfn(&basic_util<t>::default_construct), "make");
				}
				if constexpr(std::is_move_constructible_v<t>)
				{
					ret.add(sfn(&basic_util<t>::move_construct), "make");
					ret.add(sfn(&basic_util<t>::move_construct), "move-make");
				}
				if constexpr(std::is_copy_constructible_v<t>)
				{
					ret.add(sfn(&basic_util<t>::copy_construct), "make");
					ret.add(sfn(&basic_util<t>::copy_construct), "copy-make");
				}
				if constexpr(std::is_move_assignable_v<t>)
				{
					ret.add(sfn(&basic_util<t>::move_assign), "give");
					ret.add(sfn(&basic_util<t>::move_assign), "move-give");
				}
				if constexpr(std::is_copy_assignable_v<t>)
				{
					ret.add(sfn(&basic_util<t>::copy_assign), "give");
					ret.add(sfn(&basic_util<t>::copy_assign), "copy-give");
				}
				ret.add(sfn(&basic_util<t>::swap), "swap");
				return ret;
			}

			static std::string get_name(type_info_set const& from)
			{
				return name_of<val_wrap<t>>(from);
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

			static std::string get_name(type_info_set const& from)
			{
				return name_of<val_wrap<t>>(from);
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
				return std::move(basic.use("", std::move(extended)));
			}

			static std::string get_name(type_info_set const& from)
			{
				return name_of<val_wrap<t>>(from);
			}
		};

		struct core
		{
			static object_holder swap(std::vector<stack_elem>& a)
			{
				if (a.size() == 2 && (a[0].is_nullval() || a[0]->has_value()) && (a[1].is_nullval() || a[1]->has_value()))
				{
					auto o0 = get_value(a[0]);
					auto o1 = get_value(a[1]);
					set_value(a[0], std::move(o1));
					set_value(a[1], std::move(o0));
					return object_holder::make<void_object>(); //indicates a successful function call even though the return type is void
				}
				else
				{
					return object_holder::make_nullval();
				}
			}

			static object_holder first(std::vector<stack_elem>& a)
			{
				if (a.size() == 0 || (*a.begin()).is_nullval() || !(*a.begin())->has_value())
				{
					return object_holder::make_nullval();
				}
				else
				{
					return get_value(*a.begin());
				}
			}

			static object_holder last(std::vector<stack_elem>& a)
			{
				if (a.size() == 0 || (*a.rbegin()).is_nullval() || !(*a.rbegin())->has_value())
				{
					return object_holder::make_nullval();
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
				ret.add(mfn(core::swap), "swap")
					.add(mfn(core::first), "first")
					.add(mfn(core::last), "last");
				return ret;
			}

			static std::string get_name(type_info_set const&)
			{
				return "core";
			}
		};

		struct cpp_core
		{
			static object_holder strengthen(std::vector<stack_elem>& a)
			{
				if (a.size() == 1 && !a[0].is_nullval() && a[0]->is_object())
				{
					return std::move(*get_value(a[0])).as_non_trivially_destructible();
				}
				else
				{
					return object_holder::make_nullval();
				}
			}

			static object_holder drop(std::vector<stack_elem>& a)
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
				return object_holder::make<void_object>(); //indicates a successful function call even though the return type is void
			}

			static object_holder make_clone(std::vector<stack_elem>& a)
			{
				if (a.size() == 1 && !a[0].is_nullval() && a[0]->has_value())
				{
					return a[0].downcast_get<value_elem_val>()->make_clone();
				}
				else
				{
					return object_holder::make_nullval();
				}
			}

			static object_holder unwrap(std::vector<stack_elem>& a)
			{
				if (a.size() == 1 && !a[0].is_nullval() && a[0]->has_value())
				{
					return a[0].downcast_get<value_elem_val>()->unwrap();
				}
				else
				{
					return object_holder::make_nullval();
				}
			}

			static object_holder move_from_reference(std::vector<stack_elem>& a)
			{
				if (a.size() == 1 && !a[0].is_nullval() && a[0]->has_value())
				{
					return a[0].downcast_get<value_elem_val>()->take_value();
				}
				else
				{
					return object_holder::make_nullval();
				}
			}

			static object_holder make_reference_to(std::vector<stack_elem>& a)
			{
				if (a.size() == 1 && !a[0].is_nullval() && a[0]->has_value())
				{
					return a[0].downcast_get<value_elem_val>()->make_reference();
				}
				else
				{
					return object_holder::make_nullval();
				}
			}

			static object_holder make_constant_reference_to(std::vector<stack_elem>& a)
			{
				if (a.size() == 1 && !a[0].is_nullval() && a[0]->has_value())
				{
					return a[0].downcast_get<value_elem_val>()->make_constant_reference();
				}
				else
				{
					return object_holder::make_nullval();
				}
			}

			static object_holder to_string(std::vector<stack_elem>& a)
			{
				if (a.size() == 1 && !a[0].is_nullval() && a[0]->has_value())
				{
					return make_object(a[0].downcast_get<value_elem_val>()->convert_into_string());
				}
				else
				{
					return object_holder::make_nullval();
				}
			}
		};

		template<>
		struct fs_info<cpp_core>
		{
			static function_set get_functions()
			{
				function_set ret;
				ret.add(mfn(cpp_core::drop), "drop")
					.add(mfn(cpp_core::make_clone), "clone")
					.add(mfn(cpp_core::to_string), "to_str")
					.add(mfn(cpp_core::unwrap), "unwrap")
					.add(mfn(cpp_core::move_from_reference), "take")
					.add(mfn(cpp_core::make_reference_to), "ref")
					.add(mfn(cpp_core::make_constant_reference_to), "const-ref")
					.add(mfn(cpp_core::strengthen), "strong");
				return ret;
			}

			static std::string get_name(type_info_set const&)
			{
				return "cpp";
			}
		};




	}

}
