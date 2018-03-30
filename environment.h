#pragma once
#include"function_evaluations.h"


class function_set
{
public:

private:
    std::unordered_map<std::string,mu::virt<any_callable>> map;
};


class variable_value_stack
{
public:

    void push_front(value_holder&& a)
    {
        values.emplace_back(std::move(a));
    }

    std::optional<value_holder*> front()
    {
        if(values.size() == 0)
        {
            return std::nullopt;
        }
        else
        {
            return &*values.rend();
        }
    }

    std::optional<value_holder> take_front()
    {
        if(values.size() == 0)
        {
            return std::nullopt;
        }
        else
        {
            auto ret = std::move(*values.rend());
            values.pop_back();
            return std::optional<value_holder>{std::move(ret)};
        }
    }

private:
    std::vector<value_holder> values;
};

class variable_set
{
public:

    //value_holder* push_var

private:
    std::unordered_map<std::string,variable_value_stack> map;
};



class environment
{
public:

    function_set functions;

    variable_set variables;
    variable_value_stack garbage;
};