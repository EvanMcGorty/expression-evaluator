
#pragma once
#include<unordered_map>

#include"tuple_call_unpack.h"
#include"object_holding.h"

class stack
{
public:
    std::vector<mu::virt<any_elem_val>> stuff;
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
    void try_perform(stack& a) const override
    {
        assert(a.stuff.size() >= arg_len());

        arg_tuple_type to_use;
    }

    void do_call(arg_tuple_type&& a)
    {
        
    }

    using arg_tuple_type = std::tuple<std::optional<type_wrap<args>::ref_to_ptr()::typename held>...>;

    std::function<ret_t(args...)> target;
};

class environment
{
public:

    std::unordered_map<std::string,mu::virt<any_callable>> functions;

    std::unordered_map<std::string,mu::virt<any_object>> variable_values;
    std::vector<mu::virt<any_object>> garbage_values;
};