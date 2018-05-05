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
			friend void perform_all(executable&& tar, stack& loc, environment& env, std::ostream& errors);
			friend void perform(statement&& todo, stack& loc, variable_set& variables, function_set& functions, variable_value_stack& garbage, std::ostream& errors);
		public:


			stack run(executable&& a, std::ostream& errors);

			stack_elem evaluate(elem&& a, std::ostream& errors);

			void attach(std::istream& input = std::cin, std::ostream& output = std::cout, option_set&& settings = option_set{}, name_set const& names = global_type_renames)
			{
				if (!settings.default_final_operation)
				{
					settings.auto_call("_info");
				}
				function_set special_functions;
				special_functions
					.add(garbage_getter(), "garb")
					.add(info_printer(output, names), "info")
					.add(value_printer(output),"view")
					.add(variables_printer(output, names), "vars")
					.add(functions_printer(output, names), "funcs")
					.add(manual(&core::swap),"swap")
					.add(manual(&cpp_core::drop), "drop");
				while (true)
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
			}

			environment& unbind(std::string const& name)
			{
				functions.remove(name);
				return *this;
			}

			template<typename string_convertible>
			environment& rbind(string_convertible&& name, held_callable&& target)
			{
				if (!target.is_nullval())
				{
					functions.add(std::move(target), std::forward<string_convertible>(name));
				}
				return *this;
			}
			
			template<typename string_convertible, typename ret, typename...args>
			environment& fbind(string_convertible&& name, std::function<ret(args...)>&& target)
			{
				functions.add(callable(std::move(target), functions.take(std::string(name))),std::forward<string_convertible>(name));
				return *this;
			}

			template<typename string_convertible, typename ret, typename...args>
			environment& fbind(string_convertible&& name, ret(*target)(args...))
			{
				functions.add(callable(as_function(target), functions.take(std::string(name))), std::forward<string_convertible>(name));
				return *this;
			}

			template<typename string_convertible, typename member_holders_type, typename ret, typename...args>
			environment& fbind(string_convertible&& name, ret(member_holders_type::*target)(args...))
			{
				functions.add(callable(as_function(target), functions.take(std::string(name))), std::forward<string_convertible>(name));
				return *this;
			}
			template<typename string_convertible, typename member_holders_type, typename ret, typename...args>
			environment& fbind(string_convertible&& name, ret(member_holders_type::*target)(args...) const)
			{
				functions.add(callable(as_function(target), functions.take(std::string(name))), std::forward<string_convertible>(name));
				return *this;
			}
			template<typename string_convertible, typename member_holders_type, typename ret, typename...args>
			environment& fbind(string_convertible&& name, ret(member_holders_type::*target)(args...) &&)
			{
				functions.add(callable(as_function(target), functions.take(std::string(name))), std::forward<string_convertible>(name));
				return *this;
			}
			template<typename string_convertible, typename member_holders_type, typename ret, typename...args>
			environment& fbind(string_convertible&& name, ret(member_holders_type::*target)(args...) const&&)
			{
				functions.add(callable(as_function(target), functions.take(std::string(name))), std::forward<string_convertible>(name));
				return *this;
			}

			template<typename string_convertible, typename member_holders_type, typename members_type>
			environment& mbind(string_convertible&& name, members_type member_holders_type::*member)
			{
				held_callable constant_version = callable(std::function<members_type const&(member_holders_type const&)>{[pmember = member](member_holders_type const& a) -> members_type const&
				{
					return a.*pmember;
				}
				}, functions.take(std::string(name)));

				functions.add(callable(std::function<members_type&(member_holders_type&)>{[pmember = member](member_holders_type& a) -> members_type&
				{
					return a.*pmember;
				}
				}, std::optional<held_callable*>{&constant_version}), std::forward<string_convertible>(name));
				return *this;
			}


			template<typename t, typename string_convertible>
			environment& vbind(string_convertible&& name, t&& a)
			{
				functions.add(callable(std::function<t()>{[val = std::move(a)]() {return t{ val }; }}, functions.take(std::string(name))), std::forward<string_convertible>(name));
				return *this;
			}

			template<typename t = void, typename string_convertible = std::string>
			environment& sbind(string_convertible&& name = fs_name<t>(global_type_renames), function_set&& set = fs_functs<t>())
			{
				functions.use(std::move(set), std::forward<string_convertible>(name));
				return *this;
			}



			held_callable info_printer(std::ostream& to = std::cout, name_set const& from = global_type_renames)
			{
				return manual(std::function<value_holder(std::vector<stack_elem>&)>{[to = &to, from = &from](std::vector<stack_elem>& a) -> value_holder
					{
						if (a.size() == 1 && !a[0].is_nullval())
						{
							*to << a[0]->string_view(*from) << std::endl;
							return value_holder::make<void_object>(); //indicates a successful function call even though the return type is void
						}
						else
						{
							return value_holder::make_nullval();
						}
					}});
			}


			held_callable value_printer(std::ostream& to = std::cout)
			{
				return manual(std::function<value_holder(std::vector<stack_elem>&)>{[to = &to](std::vector<stack_elem>& a) -> value_holder
				{
					if (a.size() == 1 && !a[0].is_nullval())
					{
						*to << a[0]->convert_into_string() << std::endl;
						return value_holder::make<void_object>(); //indicates a successful function call even though the return type is void
					}
					else
					{
						return value_holder::make_nullval();
					}
				}});
			}
			
			held_callable garbage_getter()
			{
				return manual(std::function<value_holder(std::vector<stack_elem>&)>{
					[g = &garbage](std::vector<stack_elem>& a) -> value_holder
					{
						if (a.size() != 1 || a[0].is_nullval() || !a[0]->is_reference())
						{
							return value_holder::make_nullval();
						}

						value_holder& ret = *a[0].downcast_get<value_reference>()->ref;

						std::optional<value_holder> f = std::move(g->take_front());
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
								return value_holder::make<void_object>(); //indicates a successful function call even though the return type is void
							}
						}
						return value_holder::make_nullval();
					}
				});
			}

			held_callable functions_printer(std::ostream& to = std::cout, name_set const& names = global_type_renames)
			{
				return callable(std::function<void()>{
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
				return callable(std::function<void()>{
					[to = &to, vs = &variables,names = &names]() -> void
					{
						vs->put_values(*to,*names);
					}
				});
			}


		private:
			function_set functions;

			variable_set variables;
			variable_value_stack garbage;
		};
	}
}