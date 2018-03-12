#pragma once

#include<tuple>
#include<functional>
#include<iostream>

template<typename ret_t, typename tar_t, typename...args_t>
std::function<ret_t(args_t...)> bind(std::function<ret_t(tar_t,args_t...)>&& f, tar_t&& t)
{
    return
        [t{std::move(t)},f{std::move(f)}](args_t&&... args) -> ret_t 
        {
            return f(t, std::move(args)...);
        };
}

template<size_t ind, typename ret_t, typename tup_t, typename arg_t>
std::function<ret_t()> make_unpacked_call(std::function<ret_t(arg_t)>&& f, tup_t&& t)
{
    return
        [t{std::move(t)},f{std::move(f)}]() -> ret_t
        {
            return f(std::move(std::get<ind>(std::move(t))));
        };
}

template<size_t ind, typename ret_t, typename tup_t, typename arg_t, typename...args_t>
std::function<ret_t()> make_unpacked_call(std::function<ret_t(arg_t,args_t...)>&& f, tup_t&& t)
{
    return make_unpacked_call<ind+1,ret_t,tup_t,args_t...>
    (
        bind<ret_t,typename std::tuple_element<ind,tup_t>::type,args_t...>
        (std::move(f),std::move(std::get<ind>(t))),
        std::move(t)
    );
}






template<typename ret_t, typename...args_t>
ret_t call(std::function<ret_t(args_t...)>&& f, std::tuple<args_t...>&& t)
{
    return make_unpacked_call<0, ret_t, std::tuple<args_t...>, args_t...>(std::move(f),std::move(t))();
}

template<typename ret_t>
ret_t call(std::function<ret_t()>&& f, std::tuple<>&& t)
{
    return f();
}
