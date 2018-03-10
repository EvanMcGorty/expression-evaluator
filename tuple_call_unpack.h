#pragma once

#include<tuple>
#include<functional>
#include<iostream>

template<size_t ind, typename ret_t, typename tup_t, typename bound_t, typename...args_t>
std::function<ret_t(args_t...)> bind(std::function<ret_t(bound_t,args_t...)> f, tup_t const& t)
{
    return
        [=](args_t... args) -> ret_t 
        {
            return f(std::get<ind>(t), args...);
        };
}


template<size_t ind, typename ret_t, typename tup_t, typename bound_t, typename...args_t>
std::function<ret_t()> make_unpacked_call(std::function<ret_t(bound_t,args_t...)> f, tup_t const& t)
{
    return make_unpacked_call<ind+1,ret_t,tup_t,args_t...>(bind<ind,ret_t,tup_t,bound_t,args_t...>(f,t),t);
}

template<size_t ind, typename ret_t, typename tup_t, typename bound_t>
std::function<ret_t()> make_unpacked_call(std::function<ret_t(bound_t)> f, tup_t const& t)
{
    return
        [=]()-> ret_t
        {
            return f(std::get<ind>(t));
        };
}

template<typename ret_t, typename...args_t>
ret_t call(std::function<ret_t(args_t...)> const& f, std::tuple<args_t...> const& t)
{
    return make_unpacked_call<0, ret_t, std::tuple<args_t...>, args_t...>(f,t)();
}

template<typename ret_t>
ret_t call(std::function<ret_t()> const& f, std::tuple<> const& t)
{
    return f();
}
