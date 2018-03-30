#pragma once
#include"environment.h"

void perform(statement&& todo, stack& loc, environment& env)
{
    if(todo.val->is_literal_push())
    {
        loc.stuff.push_back(stack_elem::make<unparsed>(std::move(todo.val.downcast_get<literal_push>()->val)));
    }
    else if(todo.val->is_variable_push())
    {
        variable_push temp = std::move(*todo.val.downcast_get<variable_push>());

        switch(temp.var.change)
        {
        case(variable::sc::pop):
            stack_elem to_push = 
            break;
        }
    }
    else if(todo.val->is_function_call())
    {

    }
    else
    {
        assert(false);
    }
}