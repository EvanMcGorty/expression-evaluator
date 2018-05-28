#pragma once
#include"smart_environments.h"

namespace expr
{
	namespace impl
	{

		inline void perform(statement&& todo, stack& loc, variable_set& variables, function_set& functions, variable_value_stack& garbage, std::ostream& errors)
		{
			if (todo.val.is_nullval())
			{
				errors << "null value found\n";
				loc.stuff.emplace_back(stack_elem::make_nullval());
			}
			else if (todo.val->is_literal_push())
			{
				loc.stuff.push_back(stack_elem::make<unparsed>(std::move(todo.val.downcast_get<literal_push>()->val)));
			}
			else if (todo.val->is_variable_push())
			{
				variable_push temp = std::move(*todo.val.downcast_get<variable_push>());

				switch (temp.var.data.change)
				{
				case(sc::pop):
				{
					auto to_push = variables.take_var(temp.var.data.var_name);
					if (to_push)
					{
						loc.stuff.emplace_back(std::move(*to_push));
					}
					else
					{
						errors << "variable name \"" << temp.var.data.var_name << "\" not found\n";
						loc.stuff.emplace_back(stack_elem::make_nullval());
					}
				}
				break;
				case(sc::neutral):
				{
					auto to_push = variables.get_var(temp.var.data.var_name);
					if (to_push)
					{
						loc.stuff.emplace_back(stack_elem::make<value_reference>(*to_push));
					}
					else
					{
						errors << "variable name \"" << temp.var.data.var_name << "\" not found\n";
						loc.stuff.emplace_back(stack_elem::make_nullval());
					}
				}
				break;
				case(sc::push):
				{
					auto to_push = variables.push_var(std::move(temp.var.data.var_name));
					loc.stuff.emplace_back(stack_elem::make<value_reference>(to_push));
				}
				break;
				}
			}
			else if (todo.val->is_function_call())
			{
				function_call temp = std::move(*todo.val.downcast_get<function_call>());
				auto optional_todo = functions.get(temp.name);
				if (!optional_todo)
				{
					errors << "function name \"" << temp.name << "\" not found\n";
					garbage.clean_all_to_front(loc, temp.arg_count);
					loc.stuff.emplace_back(stack_elem::make_nullval());
				}
				else
				{
					any_callable& todo = ***optional_todo;

					object_holder tp = object_holder::make_nullval();
					bool no_exceptions = true;
					try
					{
						tp = todo.try_perform(loc, temp.arg_count);
					}
					catch (std::exception caught)
					{
						errors << "call to function \"" << temp.name << "\" threw an exception: " << caught.what() << "\n";
						tp = object_holder::make_nullval();
						no_exceptions = false;
					}

					if (no_exceptions && tp.is_nullval())
					{
						errors << "call to function \"" << temp.name << "\" returned null\n";
					}
					garbage.clean_all_to_front(loc, temp.arg_count);
					loc.stuff.emplace_back(std::move(tp));
				}
			}
			else
			{
				assert(false);
			}
		}

		inline void perform_all(executable&& tar, stack& loc, environment& env, std::ostream& errors)
		{
			for (auto&& it : std::move(tar.statements))
			{
				perform(std::move(it), loc, env.variables,env.functions, env.garbage, errors);
			}
		}

		inline stack environment::run(executable&& tar, std::ostream& errors)
		{
			stack loc;
			perform_all(std::move(tar), loc, *this, errors);
			return loc;
		}

		inline stack_elem environment::evaluate(elem&& tar, std::ostream& errors)
		{
			executable to_run;
			std::move(tar).into_executable(to_run);
			stack v = run(std::move(to_run), errors);
			assert(v.stuff.size() == 1);
			return std::move(*v.stuff.begin());
		}

	}
}