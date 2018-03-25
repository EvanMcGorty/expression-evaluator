#pragma once
#include"function_evaluations.h"

void perform(statement&& todo, stack& loc, environment& env)
{
    if(todo.val.can_downcast<literal_push>())
    {
        loc.stuff.push_back(stack_elem::make<unparsed>(std::move(todo.val.downcast_get<literal_push>())));
    }
    else if(todo.val.can_downcast<variable_push>())
    {
        variable_push temp = std::move(*todo.val.downcast_get<variable_push>());

        /*
        auto var = env.variable_values
        todo:
            reorganize environment in its own file with classes for the variable map, function map, and variable garbage.
            create member functions for doing common operations on the maps
        */
        switch(temp.var.change)
        {
        case(variable::sc::pop):
        }
    }
    else if(todo.val.can_downcast<function_call>())
    {

    }
}