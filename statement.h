#pragma once
#include"tree_structure.h"

class stack_push : public node
{
    bool is_stack_push() const override
    {
        return true;
    }

    elem duplicate() const override
    {
        assert(false); //this should not be called
    }

    bool is_equal(node const* a) const override
    {
        assert(false); //this should not be called
    }

    std::string make_string() const override
    {
        assert(false); //this should not be called
    }
};

class stack_pop : public node
{
    bool is_stack_pop() const override
    {
        return true;
    }

    elem duplicate() const override
    {
        assert(false); //this should not be called
    }

    bool is_equal(node const* a) const override
    {
        assert(false); //this should not be called
    }

    std::string make_string() const override
    {
        assert(false); //this should not be called
    }
};

class statement
{
public:
    std::string fn_name;
    std::vector<mu::algebraic<node,literal,variable,stack_push,stack_pop>> args;
};

inline void elem::push_statements(std::vector<statement>& a) const
{
    
}