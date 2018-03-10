#pragma once
#include<unordered_map>
#include<typeinfo>

#include"virtual-function-utilities\free-store_virtual.h"

#include"tuple_call_unpack.h"

#include"literal_conversions.h"

#include"statement.h"


class any_object
{
public:
    virtual std::type_info const& get_type() const = 0;

    template<typename t>
    bool is() const
    {
        return typeid(t) == get_type();
    }
};

template<typename t>
class object_of : public any_object
{
private:
    constexpr std::type_info const& type() const
    {
        return typeid(t);
    }

public:

    std::type_info const& get_type() const override
    {
        return type();
    }

    t val;
};

class any_callable
{
    virtual size_t arg_len() const = 0;
};


template<typename...args>
class callable_of : public any_callable
{
public:
    
    size_t arg_len() const
    {
        return sizeof...(args);
    }



    std::function<void(args...)> target;
};

class environment
{
public:

    std::unordered_map<std::string,mu::virt<any_callable>> functions;

    std::unordered_map<std::string,mu::virt<any_object>> variable_values;
    std::vector<mu::virt<any_object>> garbage_values;
};