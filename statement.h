#pragma once
#include"tree_structure.h"

class operation
{
public:

    virtual ~operation()
    { }

    virtual bool is_literal_push() const
    {
        return false;
    }

    virtual bool is_variable_push() const
    {
        return false;
    }


    virtual bool is_function_call() const
    {
        return false;
    }

    virtual std::string view() const = 0;

};


class literal_push : public operation
{
public:

    literal_push(literal&& a) :
        val(std::move(a))
    { }

    literal val;

    bool is_literal_push() const final override
    {
        return true;
    }

    std::string view() const override
    {
        return std::string("literal_push{") + val.make_string() + "}";
    }
};

class variable_push : public operation
{
public:

    variable_push(variable&& a) :
        var(std::move(a))
    { }

    variable var;
    
    bool is_variable_push() const final override
    {
        return true;
    }

    std::string view() const override
    {
        return std::string("variable_push{") + var.make_string() + "}";
    }
};



class function_call : public operation
{
public:

    function_call(function const& a)
    {
        name = a.data.fn_name;
        arg_count = a.data.arguments.size();
    }

    std::string name;
    size_t arg_count;

    bool is_function_call() const final override
    {
        return true;
    }

    std::string view() const override
    {
        return std::string("function_call{") + name + ", " + std::to_string(arg_count) + "}";
    }

};

class statement
{
public:

    using val_type = mu::virt<operation>;

    statement(val_type&& a) :
        val(std::move(a))
    { }

    val_type val;
};

inline void elem::push_statements(std::vector<statement>& a) const
{
    if(val.is_nullval())
    {
        return;
    }
    else if(val->is_literal())
    {
        a.push_back(statement::val_type::make<literal_push>(literal{*val.downcast_get<literal>()}));
    }
    else if(val->is_variable())
    {
        a.push_back(statement::val_type::make<variable_push>(variable{*val.downcast_get<variable>()}));
    }
    else if(val->is_function())
    {
        function const* f = val.downcast_get<function>();
        //for(std::vector<elem>::const_iterator it = f->data.arguments.cbegin(); it != f->data.arguments.cend(); ++it)
        for(auto const& it : f->data.arguments)
        //for(int i = 0; i!=f->data.arguments.size();++i)
        {
            //it->push_statements(a);
            it.push_statements(a);
            //f->data.arguments[i].push_statements(a);
        }
        a.push_back(statement::val_type::make<function_call>(function{*val.downcast_get<function>()}));
    }
}