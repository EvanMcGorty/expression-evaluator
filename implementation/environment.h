#pragma once
#include"function_sets.h"


namespace expr
{
	namespace impl
	{

		class environment;

		void perform_all(executable&& tar, stack& loc, environment& env, std::ostream& errors);
		void perform(statement&& todo, stack& loc, variable_set& variables, function_set& functions, variable_value_stack& garbage, std::ostream& errors);
		

		class environment
		{
		public:


			stack run(executable&& a, std::ostream& errors);

			stack_elem evaluate(expression&& a, std::ostream& errors);

			stack_elem evaluate(std::string const& a, std::ostream& errors)
			{
				return evaluate(expression::make(a), errors);
			}

			held_callable info_printer(std::ostream& to = std::cout, name_set const& from = global_type_renames())
			{
				return mfn(std::function<object_holder(std::vector<stack_elem>&)>{[to = &to, from = &from](std::vector<stack_elem>& a) -> object_holder
					{
						if (a.size() == 1 && !a[0].is_nullval())
						{
							if(!(a[0]->is_object() && a[0].downcast_get<any_object>()->is<void>()))
							{
								*to << a[0]->string_view(*from) << std::endl;
							}
							return object_holder::make<void_object>(); //indicates a successful function call even though the return type is void
						}
						else
						{
							return object_holder::make_nullval();
						}
					}});
			}


			held_callable value_printer(std::ostream& to = std::cout)
			{
				return mfn(std::function<object_holder(std::vector<stack_elem>&)>{[to = &to](std::vector<stack_elem>& a) -> object_holder
				{
					if (a.size() == 1 && !a[0].is_nullval())
					{
						if(!(a[0]->is_object() && a[0].downcast_get<any_object>()->is<void>()))
						{
							*to << a[0]->convert_into_string() << std::endl;
						}
						return object_holder::make<void_object>(); //indicates a successful function call even though the return type is void
					}
					else
					{
						return object_holder::make_nullval();
					}
				}});
			}
			
			held_callable garbage_getter()
			{
				return mfn(std::function<object_holder(std::vector<stack_elem>&)>{
					[g = &garbage](std::vector<stack_elem>& a) -> object_holder
					{
						if (a.size() != 1 || a[0].is_nullval() || !a[0]->is_reference())
						{
							return object_holder::make_nullval();
						}

						object_holder& ret = *a[0].downcast_get<value_reference>()->ref;

						std::optional<object_holder> f = g->take_front();
						while (f)
						{
							if (f->is_nullval())
							{
								f = g->take_front();
								continue;
							}
							else
							{
								std::swap(*f,ret);
								return object_holder::make<void_object>(); //indicates a successful function call even though the return type is void
							}
						}
						return object_holder::make_nullval();
					}
				});
			}

			held_callable functions_printer(std::ostream& to = std::cout, name_set const& names = global_type_renames())
			{
				return sfn(std::function<void()>{
					[to = &to,fs = &functions, names = &names]() -> void
					{
						for (auto const& it : fs->view())
						{
							*to << it.first;
							it.second->put_type(*to,*names);
							*to << '\n' << std::flush;
						}
					}
				});
			}

			held_callable variables_printer(std::ostream& to = std::cout, name_set const& names = global_type_renames())
			{
				return sfn(std::function<void()>{
					[to = &to, vs = &variables,names = &names]() -> void
					{
						vs->put_values(*to,*names);
					}
				});
			}


			function_set functions;

			variable_set variables;
			variable_value_stack garbage;
		};
	}
}
