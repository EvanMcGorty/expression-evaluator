#pragma once
#include"environment.h"


void stack::clear_front(size_t a, variable_value_stack& garbage)
{
    assert(a <= stuff.size());

    for (int i = 0; i != a; ++i)
    {
        if (!stuff.rbegin()->is_nullval() && (*stuff.rbegin())->is_object())
        {
            garbage.push_front(std::move(std::move(*stuff.rbegin()).downcast<any_object>()));
        }
        stuff.pop_back();
    }
}

void perform(statement&& todo, stack& loc, environment& env)
{
    if(todo.val->is_literal_push())
    {
        loc.stuff.push_back(stack_elem::make<unparsed>(std::move(todo.val.downcast_get<literal_push>()->val)));
    }
    else if(todo.val->is_variable_push())
    {
        variable_push temp = std::move(*todo.val.downcast_get<variable_push>());

        switch(temp.var.data.change)
        {
        case(sc::pop):
        {
            auto to_push = env.variables.take_var(std::move(temp.var.data.var_name));
            if(to_push)
            {
                loc.stuff.emplace_back(std::move(*to_push));
            }
            else
            {
                loc.stuff.emplace_back(stack_elem::make_nullval());
            }
        }
        break;
        case(sc::neutral):
        {
            auto to_push = env.variables.get_var(std::move(temp.var.data.var_name));
            if(to_push)
            {
                loc.stuff.emplace_back(stack_elem::make<value_reference>(*to_push));
            }
            else
            {
                loc.stuff.emplace_back(stack_elem::make_nullval());
            }
        }
        break;
        case(sc::push):
        {
            auto to_push = env.variables.push_var(std::move(temp.var.data.var_name));
            loc.stuff.emplace_back(stack_elem::make<value_reference>(to_push));
        }
        break;
        }
    }
    else if(todo.val->is_function_call())
    {
        function_call temp = std::move(*todo.val.downcast_get<function_call>());
        auto optional_todo = env.functions.get(temp.name);
        if(!optional_todo)
        {
            loc.clear_front(temp.arg_count,env.garbage);
			loc.stuff.emplace_back(stack_elem::make_nullval());
        }
        else
        {
            any_callable& todo = ***optional_todo;

            if(todo.arg_len() == temp.arg_count)
            {
                value_holder tp = todo.try_perform(loc);
                loc.clear_front(todo.arg_len(),env.garbage);
                loc.stuff.emplace_back(std::move(tp));
            }
            else if(todo.arg_len() + 1 == temp.arg_count && (**(loc.stuff.rbegin()+todo.arg_len())).is_reference())
            {
                value_holder tp = todo.try_perform(loc);
                loc.clear_front(todo.arg_len(),env.garbage);
                *loc.stuff.rbegin()->downcast_get<value_reference>()->ref = std::move(tp);
            }
            else
            {
                loc.clear_front(temp.arg_count,env.garbage);
				loc.stuff.emplace_back(stack_elem::make_nullval());
            }
        }
    }
    else
    {
        assert(false);
    }
}

void perform_all(std::vector<statement>&& executable, stack& loc, environment& env)
{
    for(auto&& it : executable)
    {
        perform(std::move(it),loc,env);
    }
    loc.clear_front(loc.stuff.size(),env.garbage);
}