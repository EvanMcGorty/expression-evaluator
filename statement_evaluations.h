#pragma once
#include"function_evaluations.h"

void perform(statement&& todo, stack& loc, environment& env)
{
    if(todo.val.can_downcast<literal_push>())
    {
        std::move(todo.val.downcast_get<literal_push>());
    }
    else if(todo.val.can_downcast<variable_push>())
    {

    }
    else if(todo.val.can_downcast<function_call>())
    {

    }
}