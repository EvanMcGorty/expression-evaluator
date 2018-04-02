#pragma once

#include<tuple>
#include<iostream>
#include<typeinfo>
#include<functional>

//todo: try making these constexpr

namespace expressions
{

	template<typename ret_t, typename funct_t, typename tar_t, typename...args_t>
	constexpr auto perfect_bind(funct_t&& f, tar_t&& tar)
	{
		return
			[b = std::move(tar), f = std::move(f)](args_t&&... args) mutable->ret_t
		{
			return f(std::move(b), std::move(args)...);
		};
	}

	template<typename ret_t, typename funct_t, typename tuple_t, size_t index, typename argf_t, typename...args_t>
	constexpr auto perfect_bind_all(funct_t&& f, tuple_t&& tup)
	{
		if constexpr(sizeof...(args_t) == 0)
		{
			return perfect_bind<ret_t, funct_t, argf_t>(std::move(f), std::move(std::get<index>(std::move(tup))));
		}
		else
		{
			auto newfn = perfect_bind<ret_t, funct_t, argf_t, args_t...>(std::move(f), std::move(std::get<index>(std::move(tup))));
			return perfect_bind_all<ret_t, decltype(newfn), tuple_t, index + 1, args_t...>(std::move(newfn), std::move(tup));
		}
	}



	template<typename ret, typename...args>
	constexpr ret call(std::function<ret(args...)>&& f, std::tuple<typename std::remove_reference<args>::type...>&& t)
	{
		return perfect_bind_all<ret, std::function<ret(args...)>, std::tuple<typename std::remove_reference<args>::type...>, 0, args...>(std::move(f), std::move(t))();
	}

	template<typename ret, typename...args>
	constexpr ret call(ret(*f)(args...), std::tuple<typename std::remove_reference<args>::type...>&& t)
	{
		return perfect_bind_all<ret, ret(*)(args...), std::tuple<typename std::remove_reference<args>::type...>, 0, args...>(std::move(f), std::move(t))();
	}

}