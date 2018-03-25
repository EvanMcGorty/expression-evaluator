#pragma once
#include"literal_conversions.h"

#include<typeinfo>
#include<any>


class any_type_ask
{
public:
    virtual void parse(std::string const& a, std::any& b) = 0;

    virtual std::type_info get_type() const = 0;
};

template<typename t>
class type_ask_of : public any_type_ask
{
public:

    void parse(std::string const& a, std::any& b) override
    {
        if constexpr(std::is_pointer<t>::value)
        {
            auto temp = convert<std::remove_const<typename std::remove_pointer<t>::type>::type>(a);
            if(temp)
            {
                b = std::move(*temp);
            }
            gotten = std::optional<t>{std::any_cast<std::remove_const<typename std::remove_pointer<t>::type>::type>(&b)};
        }
        else
        {
            gotten = convert<t>(a);
        }
    }

    std::type_info get_type() const
    {
        return typeid(t);
    }

    std::optional<t> gotten;
};