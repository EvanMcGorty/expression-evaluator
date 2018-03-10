#pragma once
#include"tree_structure.h"

class statement
{
public:
    std::string fn_name;
    mu::algebraic<node,literal,variable> args;
};

inline void elem::push_statements(std::vector<statement> a) const
{
    
}