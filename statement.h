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

};

class literal_push : public operation
{
public:

    literal_push(literal&& a) :
        val(std::move(a))
    { }

    literal val;

    bool is_literal_push() const override
    {
        return true;
    }
};

class variable_push : public operation
{
public:

    variable_push(variable&& a) :
        var(std::move(a))
    { }

    variable var;
    
    bool is_variable_push() const override
    {
        return true;
    }
};

class function_call : public operation
{
public:

    function_call(function const& a)
    {
        name = a.view_fn_name();
        arg_count = a.view_arguments().size();
    }

    std::string name;
    size_t arg_count;

    bool is_function_call() const override
    {
        return true;
    }

};

class statement
{
public:

    using val_type = mu::algebraic<operation,literal_push,variable_push,function_call>;

    statement(val_type&& a) :
        val(std::move(a))
    { }

    val_type val;
};

inline void elem::push_statements(std::vector<statement>& a) const
{
    if(val->is_literal())
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
        for(auto it = f->view_arguments().begin(); it != f->view_arguments().end(); ++it)
        {
            it->push_statements(a);
        }
        a.push_back(statement::val_type::make<function_call>(function{*val.downcast_get<function>()}));
    }
}