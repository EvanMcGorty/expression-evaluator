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

		if constexpr (std::template is_rvalue_reference<t>::value || std::template is_lvalue_reference<t>::value)
		{
			return type_wrap<t>();
		}
		else
		{
			return type_wrap<t const&>();
		}
	}

	template<typename t>
	constexpr auto as_returnable()
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
			return type_wrap<t>();
		}
	}

	//msvc 17 needs this goofy workaround in order to compile.

	template<typename t>
	struct store
	{
		typedef decltype(as_storable<t>()) type;
	};

	template<typename t>
	using store_t = typename store<t>::type::held;


	template<typename t>
	struct pass
	{
		typedef decltype(as_passable<t>()) type;
	};

	template<typename t>
	using pass_t = typename pass<t>::type::held;

	template<typename t>
	struct returned
	{
		typedef decltype(as_returnable<t>()) type;
	};

	template<typename t>
	using return_t = typename returned<t>::type::held;


	template<typename t>
	constexpr pass_t<t> storable_into_passable(store_t<t>&& x)
	{
		if constexpr(std::is_rvalue_reference<t>::value && !std::is_const<typename std::remove_reference<t>::type>::value)
		{
			return std::move(x);
		}
		else if constexpr(!std::is_reference<t>::value)
		{
			return std::move(*x);
		}
		else if constexpr(std::is_lvalue_reference<t>::value)
		{
			return *x;
		}
		else
		{
			static_assert(false, "template meta-programming error");
		}
	}

	template<typename t>
	constexpr return_t<t> into_returnable(t&& x)
	{
		if constexpr( !std::is_reference<t>::value || (std::is_rvalue_reference<t>::value && !std::is_const<typename std::remove_reference<t>::type>::value))
		{
			return std::move(x);
		}
		else if constexpr(std::is_lvalue_reference<t>::value)
		{
			return &x;
		}
		else
		{
			static_assert(false, "template meta-programming error");
		}
	}


	template<typename ret_t, typename...argts>
	std::function<return_t<ret_t>(store_t<argts>...)> make_storable_call(std::function<ret_t(argts...)>&& f)
	{
		if constexpr(!std::is_void<ret_t>::value)
		{
			return std::function<return_t<ret_t>(store_t<argts>&&...)> {
				[f = std::move(f)](store_t<argts>...argvs)->return_t<ret_t>
				{
					return into_returnable<ret_t>(f(storable_into_passable<argts>(std::move(argvs))...));
				}
			};
		}
		else
		{
			return std::function<void(store_t<argts>...)> {
				[f = std::move(f)](store_t<argts>&&...argvs) -> void
				{
					f(storable_into_passable<argts>(std::move(argvs))...);
				}
			};
		}
	}
}