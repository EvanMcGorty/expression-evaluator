#pragma once
#include<type_traits>
#include<utility>
#include<functional>


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

//remove reference is not nessecary for actual logic, it is only needed for vc17
template<typename t>
using store_t = typename std::remove_reference_t<decltype(as_storable<t>())>::held;

template<typename t>
constexpr auto move_or_deref(store_t<t>& x)
{
    if constexpr(std::is_rvalue_reference<t>::value && !std::is_const<typename std::remove_reference<t>::type>::value)
    {
        return std::move(x);
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