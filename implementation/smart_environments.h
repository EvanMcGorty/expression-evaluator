#include"environment.h"

namespace expr
{
	namespace impl
	{


		struct option_set
		{
			friend class interpreter;
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


		class interpreter : public environment
		{
		public:

			interpreter(environment&& base = environment{}, std::istream& i = std::cin, std::ostream& o = std::cout, option_set&& s = option_set{}, type_info_set const& n = global_type_info()) :
				environment(std::move(base)),
				input(i),
				output(o),
				settings(std::move(s)),
				names(n)
			{
				if (!settings.default_final_operation)
				{
					settings.auto_call("_info");
				}

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
			}

			void once()
			{
				output << "\\\\\\\n" << std::flush;

				std::string unparsed;
				std::getline(input, unparsed);

				std::optional<statement> special_call;

				executable to_run;

				prepare_input(unparsed, special_call, to_run);
				
				output << "///" << std::endl;

				std::stringstream errors;

				stack result;

				evaluate_input(result, errors, to_run, special_call);

				if (settings.whether_to_print_error_messages)
				{
					output << "|||\n" << errors.str() << std::flush;
				}
			}

			void go()
			{
				to_continue = true;
				output << "/|\\" << std::endl;
				while (to_continue)
				{
					once();
				}
				output << "\\|/" << std::endl;
			}

			function_set special_functions;

			bool to_continue;

			std::istream& input;
			std::ostream& output;
			option_set settings;
			type_info_set const& names;

		private:

			void prepare_input(std::string& unparsed, std::optional<statement>& special_call, executable& to_run)
			{
				if (unparsed.begin() != unparsed.end() && *unparsed.begin() == '_')
				{
					expression n = expression::make(std::string{ unparsed.begin() + 1, unparsed.end() });
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
					expression n = expression::make(std::string{ unparsed.begin(), unparsed.end() });

					if (settings.whether_to_reprint_input_as_parsed)
					{
						output << "|||\n" << n.str() << "\n";
					}

					std::move(n).into_executable(to_run);
				}

			}

			void evaluate_input(stack& result, std::ostream& info, executable& to_run, std::optional<statement>& special_call)
			{
				result = run(std::move(to_run), info);
				if (special_call)
				{
					perform(std::move(*special_call), result, variables, special_functions, garbage, info);
				}
				else if (settings.default_final_operation)
				{
					std::string to_use = *settings.default_final_operation;
					assert_with_generic_logic_error(to_use.size() >= 1);
					if (*to_use.begin() == '_')
					{
						to_use = std::string{ to_use.begin() + 1,to_use.end() };
						perform(statement::val_type::make<function_call>(to_use, size_t(1)), result, variables, special_functions, garbage, info);
					}
					else
					{
						perform(statement::val_type::make<function_call>(to_use, size_t(1)), result, variables, functions, garbage, info);
					}
				}

				assert_with_generic_logic_error(result.stuff.size() == 1);

				garbage.clean_all_to_front(result, 1, info);
			}
			
		};

	}
}
