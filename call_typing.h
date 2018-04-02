#pragma once
#include<type_traits>
#include<utility>
#include<functional>

namespace expressions
{

	template<typename t>
	class type_wrap
	{
	public:
		type_wrap()
		{}


		typedef t held;
	};

	template<typename t>
	constexpr auto as_storable()
	{

		if constexpr (std::template is_rvalue_reference<t>::value)
		{
			if constexpr (std::is_const<typename std::remove_reference<t>::type>::value)
			{
				return type_wrap<typename std::remove_reference<t>::type const *>();
			}
			else
			{
				return type_wrap<typename std::remove_reference<t>::type>();
			}
		}
		else if constexpr (std::template is_lvalue_reference<t>::value)
		{
			if constexpr (std::is_const<typename std::remove_reference<t>::type>::value)
			{
				return type_wrap<typename std::remove_reference<t>::type const *>();
			}
			else
			{
				return type_wrap<typename std::remove_reference<t>::type *>();
			}
		}
		else
		{
			return type_wrap<typename std::add_const<t>::type *>();
		}
	}

	template<typename t>
	constexpr auto as_passable()
	{

		if constexpr (std::template is_rvalue_reference<t>::value || std::template is_rvalue_reference<t>::value)
		{
			return type_wrap<t>();
		}
		else
		{
			return type_wrap<t const&>();
		}
	}



	//msvc 17 needs this goofy workaround in order to compile.

	template<typename t>
	struct store_c
	{
		typedef decltype(as_storable<t>()) w;
	};

	template<typename t>
	using store_t = typename store_c<t>::w::held;


	template<typename t>
	struct pass_c
	{
		typedef decltype(as_passable<t>()) w;
	};

	template<typename t>
	using pass_t = typename pass_c<t>::w::held;




	template<typename t>
	constexpr pass_t<t> move_or_deref(store_t<t>& x)
	{
		if constexpr(std::is_rvalue_reference<t>::value && !std::is_const<typename std::remove_reference<t>::type>::value)
		{
			return std::move(x);
		}
		else if constexpr(!std::is_reference<t>::value)
		{
			return std::move(*x);
		}
		else
		{
			return *x;
		}
	}

	template<typename ret_t, typename...argts>
	std::function<ret_t(store_t<argts>...)> make_storable_call(ret_t(*f)(argts...))
	{
		return std::function<ret_t(store_t<argts>...)> {
			[=](store_t<argts>...argvs) -> ret_t
			{
				return f(move_or_deref<argts>(argvs)...);
			}
		};
	}
}