#pragma once
#include"function_sets.h"


namespace expr
{
	namespace impl
	{

		struct option_set
		{
			friend class environment;
			option_set()
			{
				default_final_operation = std::nullopt;
				whether_to_reprint_input_as_parsed = true;
				whether_to_print_error_messages = true;
			}

			void auto_call(std::optional<std::string>&& a)
			{
				if (a)
				{
					default_final_operation = std::string{ std::move(*a) };
				}
				else
				{
					default_final_operation = std::nullopt;
				}
			}

			void input_reprint(bool a)
			{
				whether_to_reprint_input_as_parsed = a;
			}

			void error_print(bool a)
			{
				whether_to_print_error_messages = a;
			}

		private:
			std::optional<std::string> default_final_operation;
			bool whether_to_reprint_input_as_parsed;
			bool whether_to_print_error_messages;
		};

		class environment;

		void perform_all(executable&& tar, stack& loc, environment& env, std::ostream& errors);
		void perform(statement&& todo, stack& loc, variable_set& variables, function_set& functions, variable_value_stack& garbage, std::ostream& errors);
		

		class environment
		{
		public:


			stack run(executable&& a, std::ostream& errors);

			stack_elem evaluate(elem&& a, std::ostream& errors);

			void attach(std::istream& input = std::cin, std::ostream& output = std::cout, option_set&& settings = option_set{}, name_set const& names = global_type_renames)
			{
				if (!settings.default_final_operation)
				{
					settings.auto_call("_info");
				}

				bool to_continue = true;

				function_set special_functions;
				special_functions
					.add(garbage_getter(), "garb")
					.add(info_printer(output, names), "info")
					.add(value_printer(output), "view")
					.add(variables_printer(output, names), "vars")
					.add(functions_printer(output, names), "funcs")
					.add(mfn(&core::swap), "swap")
					.add(mfn(&cpp_core::drop), "drop")
					.add(sfn(std::function<void()>{
					[to_continue = &to_continue]() {*to_continue = false; }
					}), "exit");

				output << "/|\\" << std::endl;
				while (to_continue)
				{
					output << "\\\\\\\n" << std::flush;
					std::string unparsed;
					std::getline(input, unparsed);
					std::optional<statement> special_call;
					executable to_run;
					if (unparsed.begin() != unparsed.end() && *unparsed.begin() == '_')
					{
						elem n = elem::make(std::string{unparsed.begin()+1, unparsed.end()});
						if (n.get_function() == nullptr)
						{
							if (settings.whether_to_reprint_input_as_parsed)
							{
								output << "|||\n_\n";
							}
							
							to_run.statements.push_back(statement::val_type::make_nullval());
						}
						else
						{
							if (settings.whether_to_reprint_input_as_parsed)
							{
								output << "|||\n_" << n.str() << "\n";
							}
							std::move(n).into_executable(to_run);
							special_call = std::make_optional(std::move(*to_run.statements.rbegin()));
							to_run.statements.pop_back();
						}
					}
					else
					{
						elem n = elem::make(std::string{unparsed.begin(), unparsed.end()});

						if (settings.whether_to_reprint_input_as_parsed)
						{
							output << "|||\n" << n.str() << "\n";
						}

						std::move(n).into_executable(to_run);
					}

					output << "///" << std::endl;
					std::stringstream errors;
					stack result = run(std::move(to_run), errors);
					if (special_call)
					{
						perform(std::move(*special_call), result, variables, special_functions, garbage, errors);
					}
					else if (settings.default_final_operation)
					{
						std::string to_use = *settings.default_final_operation;
						assert(to_use.size() >= 1);
						if (*to_use.begin() == '_')
						{
							to_use = std::string{ to_use.begin() + 1,to_use.end() };
							perform(statement::val_type::make<function_call>(to_use, 1), result, variables, special_functions, garbage, errors);
						}
						else
						{
							perform(statement::val_type::make<function_call>(to_use, 1), result, variables, functions, garbage, errors);
						}
					}

					assert(result.stuff.size() == 1);

					garbage.clean_all_to_front(result,1);
					
					if (settings.whether_to_print_error_messages)
					{
						output << "|||\n" << errors.str() << std::flush;
					}
				}
				output << "\\|/" << std::endl;
			}

			held_callable info_printer(std::ostream& to = std::cout, name_set const& from = global_type_renames)
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

						std::optional<object_holder> f = std::move(g->take_front());
						while (f)
						{
							if (f->is_nullval())
							{
								f = std::move(g->take_front());
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

			held_callable functions_printer(std::ostream& to = std::cout, name_set const& names = global_type_renames)
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

			held_callable variables_printer(std::ostream& to = std::cout, name_set const& names = global_type_renames)
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