
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
        if(stuff[stuff.size()-(ind+1)]->get(ask))
        {
            stuff[stuff.size()-(ind+1)] = mu::virt<any_elem_val>::make_nullval();
        }
        std::get<ind>(a) = gotten;
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
public:
    
    size_t arg_len() const override
    {
        // if(std::is_same<ret_t,void>::value)
        // {
        return sizeof...(args) + 1;
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
        a.set_from_front<arg_tuple_type,typename decltype(as_storable<args>())::held...>();
        //unfinished
    }


private:


    using arg_tuple_type = std::tuple<std::optional<typename decltype(as_storable<args>())::held>...>;


    ret_t do_call(arg_tuple_type&& a)
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