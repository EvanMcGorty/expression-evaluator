#pragma once
#include"environment.h"

namespace expr
{
	namespace impl
	{

		void stack::clear_front(size_t a, variable_value_stack& garbage)
		{
			assert(a <= stuff.size());

			for (int i = 0; i != a; ++i)
			{
				if (!stuff.rbegin()->is_nullval() && (*stuff.rbegin())->is_object())
				{
					auto to_push = std::move(std::move(*stuff.rbegin()).downcast<any_object>());
					if (!to_push->can_trivially_destruct())
					{
						garbage.push_front(std::move(to_push));
					}
				}
				stuff.pop_back();
			}
		}

		void perform(statement&& todo, stack& loc, environment& env, std::ostream& errors)
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
					auto to_push = env.variables.take_var(temp.var.data.var_name);
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
					auto to_push = env.variables.get_var(temp.var.data.var_name);
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
					auto to_push = env.variables.push_var(std::move(temp.var.data.var_name));
					loc.stuff.emplace_back(stack_elem::make<value_reference>(to_push));
				}
				break;
				}
			}
			else if (todo.val->is_function_call())
			{
				function_call temp = std::move(*todo.val.downcast_get<function_call>());
				auto optional_todo = env.functions.get(temp.name);
				if (!optional_todo)
				{
					errors << "function name \"" << temp.name << "\" not found\n";
					loc.clear_front(temp.arg_count, env.garbage);
					loc.stuff.emplace_back(stack_elem::make_nullval());
				}
				else
				{
					any_callable& todo = ***optional_todo;

					value_holder tp = todo.try_perform(loc, temp.arg_count);
					if (tp.is_nullval())
					{
						errors << "call to function \"" << temp.name << "\" returned null\n";
					}
					loc.clear_front(temp.arg_count, env.garbage);
					loc.stuff.emplace_back(std::move(tp));
				}
			}
			else
			{
				assert(false);
			}
		}

		void perform_all(executable&& tar, stack& loc, environment& env, std::ostream& errors)
		{
			for (auto&& it : std::move(tar.statements))
			{
				perform(std::move(it), loc, env, errors);
			}
			loc.clear_front(loc.stuff.size(), env.garbage);
		}

		void environment::run(executable&& tar, std::ostream& errors)
		{
			stack loc;
			perform_all(std::move(tar), loc, *this, errors);
		}

		void environment::run(elem&& tar, std::ostream& errors)
		{
			executable to_run;
			std::move(tar).into_executable(to_run);
			run(std::move(to_run), errors);
		}

	}
}