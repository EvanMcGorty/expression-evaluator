
#pragma once
#include<unordered_map>

#include"tuple_call_unpack.h"
#include"call_typing.h"
#include"object_holding.h"

class stack
{
    template<typename tup_t, size_t ind, typename t, typename...ts>
    void set_rest(tup_t& a)
    {
        type_ask_of<t> ask;
        if(stuff[stuff.size()-(ind+1)].is_nullval())
        {
            std::get<ind>(a) = std::nullopt;
        }
        if(stuff[stuff.size()-(ind+1)]->get(ask))
        {
            stuff[stuff.size()-(ind+1)] = mu::virt<any_elem_val>::make_nullval();
        }
        std::get<ind>(a) = aks.gotten;
        if(sizeof...(ts) > 0)
        {
            set_rest<tup_t,ind+1,ts...>(a);
        }
    }

public:
    std::vector<mu::virt<any_elem_val>> stuff;

    template<typename tup_t, typename...ts>
    void set_from_front(tup_t& a)
    {
        static_assert(std::is_same<tup_t,std::tuple<std::optional<ts>...>>::value);
        set_rest<tup_t,0,ts...>(a);
    }


};


class any_callable
{
    virtual size_t arg_len() const = 0;
    virtual void try_perform(stack& a) const = 0;
};



template<typename ret_t,typename...args>
class callable_of : public any_callable
{
    using use_tuple_type = std::tuple<typename decltype(as_storable<args>())::held...>;
    using arg_tuple_type = std::tuple<std::optional<typename decltype(as_storable<args>())::held>...>;
public:
    
    size_t arg_len() const override
    {
        // if(std::is_same<ret_t,void>::value)
        // {
        return sizeof...(args);
        // }
        // else
        // {
        //     return sizeof...(args);
        // }
    }

    //when the stack is not popped from, it is the callers responsibility to manage garbage variables
    std::optional<ret_t> try_perform(stack& a) const override
    {
        assert(a.stuff.size() >= arg_len());
        arg_tuple_type to_use;
        a.set_from_front<arg_tuple_type,typename decltype(as_storable<args>())::held...>(to_use);
        std::optional<use_tuple_type> might_use;
        can_perform(might_use,std::move(to_use));
        if(might_use)
        {
            return std::optional<ret_t>{do_call(std::move(*might_use)));
        }
        else
        {
            return std::nullopt;
        }
    }

    


private:

    template<size_t ind = 0>
    static void can_perform(std::optional<use_tuple_type>& loc, arg_tuple_type&& tar)
    {
        if constexpr(ind == std::tuple_size<use_tuple_type>)
        {
            return;
        }
        else
        {
            if(std::get<ind>(tar))
            {
                std::get<ind>(*loc) = std::move(*std::get<ind>(tar));
                can_perform<ind+1>(loc,std::move(tar));
            }
            else
            {
                loc = std::nullopt;
            }
        }
    }

    ret_t do_call(use_tuple_type&& a) const
    {
        return call(target,std::move(a));
    }


    std::function<ret_t(args...)> target;
};

class environment
{
public:

    std::unordered_map<std::string,mu::virt<any_callable>> functions;

    std::unordered_map<std::string,mu::virt<any_object>> variable_values;
    std::vector<mu::virt<any_object>> garbage_values;
};