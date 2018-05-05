#pragma once
#include"function_evaluations.h"


namespace expr
{
	namespace impl
	{

		template<typename ret, typename...args>
		held_callable callable(std::function<ret(args...)>&& target, std::optional<held_callable>&& old = std::nullopt)
		{
			if (old)
			{
				return held_callable::make<multi_callable_of<ret, args...>>(std::move(target), std::move(*old));
			}
			else
			{
				return held_callable::make<callable_of<ret, args...>>(std::move(target));
			}
		}

		template<typename ret, typename...args>
		auto as_function(ret(*target)(args...))
		{
			return std::function<ret(args...)>{target};
		}

		template<typename member_holders_type, typename ret, typename...args>
		auto as_function(ret(member_holders_type::*target)(args...))
		{
			typedef member_holders_type& real_type;
			return std::function<ret(real_type, args...)>{ target };
		}

		template<typename member_holders_type, typename ret, typename...args>
		auto as_function(ret(member_holders_type::*target)(args...) const)
		{
			typedef member_holders_type const& real_type;
			return std::function<ret(real_type, args...)>{ target };
		}

		template<typename member_holders_type, typename ret, typename...args>
		auto as_function(ret(member_holders_type::*target)(args...) &&)
		{
			typedef member_holders_type&& real_type;
			return  std::function<ret(real_type, args...)>{ target };
		}

		template<typename member_holders_type, typename ret, typename...args>
		auto as_function(ret(member_holders_type::*target)(args...) const&&)
		{
			typedef member_holders_type const&& real_type;
			return std::function<ret(real_type, args...)>{ target };
		}

		held_callable manual(std::function<value_holder(std::vector<stack_elem>&)>&& target)
		{
			return held_callable::make<manual_callable>(std::move(target));
		}

		held_callable manual(value_holder(*target)(std::vector<stack_elem>&))
		{
			return held_callable::make<manual_callable>(std::function<value_holder(std::vector<stack_elem>&)>{target});
		}


		class function_set
		{
		public:

			function_set& remove(std::string const& b)
			{
				map.erase(b);
				return *this;
			}

			function_set& add(held_callable&& f, std::string&& n)
			{
				assert(name_checker::is_valid(n));
				add_to_map(std::make_pair(std::move(n), std::move(f)));
				return *this;
			}

			function_set& add(held_callable&& f, std::string const& n)
			{
				assert(name_checker::is_valid(n));
				add_to_map(std::make_pair(n, std::move(f)));
				return *this;
			}
			
			function_set& add(held_callable&& f, char const* n)
			{
				assert(name_checker::is_valid(n));
				add_to_map(std::make_pair(std::string{n}, std::move(f)));
				return *this;
			}

			function_set& use(function_set&& w, std::string const& n)
			{
				if (n == "")
				{
					return merge(std::move(w));
				}
				assert(name_checker::is_valid(n));
				for (auto it = w.map.begin(); it != w.map.end(); ++it)
				{
					auto cur = std::move(*it);
					add_to_map(std::make_pair(std::string{ n + "." + cur.first }, std::move(cur.second)));
				}
				return *this;
			}

			function_set& use(function_set&& w, char const* n)
			{
				if (std::string(n) == "")
				{
					return merge(std::move(w));
				}
				assert(name_checker::is_valid(n));
				for (auto it = w.map.begin(); it != w.map.end(); ++it)
				{
					auto cur = std::move(*it);
					add_to_map(std::make_pair(std::string{ std::string{n} +"." + cur.first }, std::move(cur.second)));
				}
				return *this;
			}

			function_set& merge(function_set&& w)
			{
				for (auto it = w.map.begin(); it != w.map.end(); ++it)
				{
					add_to_map(std::move(*it));
				}
				return *this;
			}


			std::optional<held_callable*> get(std::string const& a)
			{
				auto g = map.find(a);
				if (g == map.end())
				{
					return std::nullopt;
				}
				else
				{
					return std::optional<held_callable*>{&g->second};
				}
			}

			std::optional<held_callable> take(std::string const& a)
			{
				auto g = map.find(a);
				if (g == map.end())
				{
					return std::nullopt;
				}
				else
				{
					auto ret = std::optional<held_callable>{std::move(g->second)};
					map.erase(g);
					return std::move(ret);
				}
			}

			std::unordered_map<std::string, held_callable> const& view() const
			{
				return map;
			}

		private:

			void add_to_map(std::pair<std::string, held_callable>&& a)
			{
				auto g = map.find(std::move(a.first));
				if (g!=map.end())
				{
					g->second = std::move(*a.second).add_layer(std::move(g->second));
				}
				else
				{
					map.emplace(std::move(a.first), std::move(a.second));
				}
			}

			std::unordered_map<std::string, held_callable> map;
		};


		class variable_value_stack
		{
		public:

			value_holder * push_front(value_holder&& a)
			{
				values.emplace_back(std::move(a));
				return &*values.rbegin();
			}

			std::optional<value_holder*> get_front()
			{
				if (values.size() == 0)
				{
					return std::nullopt;
				}
				else
				{
					return &*values.rbegin();
				}
			}

			std::optional<value_holder> take_front()
			{
				if (values.size() == 0)
				{
					return std::nullopt;
				}
				else
				{
					auto ret = std::move(*values.rbegin());
					values.pop_back();
					return std::optional<value_holder>{std::move(ret)};
				}
			}

			std::string string_view(name_set const& from) const
			{
				if (values.size() == 0)
				{
					return "[]";
				}
				std::string ret{ "[" };
				for (auto const& it : values)
				{
					if (it.is_nullval())
					{
						ret.push_back('#');
					}
					else
					{
						ret.append(it->string_view(from));
					}
					ret.push_back(',');
				}
				*ret.rbegin() = ']';
				return ret;
			}

			void clean_to_front(stack_elem& from)
			{
				if (!from.is_nullval() && from->is_object())
				{
					auto to_push = std::move(std::move(from).downcast<any_object>());
					if (!to_push->can_trivially_destruct())
					{
						push_front(std::move(to_push));
					}
				}
			}

			void clean_all_to_front(stack& from, size_t count)
			{
				assert(count <= from.stuff.size());

				for (int i = 0; i != count; ++i)
				{
					clean_to_front(*from.stuff.rbegin());
					from.stuff.pop_back();
				}
			}

		private:
			std::vector<value_holder> values;
		};

		class variable_set
		{
			friend class environment;
		public:


			value_holder* push_var(std::string&& a)
			{
				return map[a].push_front(value_holder::make_nullval());
			}

			std::optional<value_holder*> get_var(std::string const& a)
			{
				auto it = map.find(a);
				if (it == map.end())
				{
					return std::nullopt;
				}
				else
				{
					return it->second.get_front();
				}
			}

			std::optional<value_holder> take_var(std::string const& a)
			{
				auto it = map.find(a);
				if (it == map.end())
				{
					return std::nullopt;
				}
				else
				{
					return std::optional<value_holder>{it->second.take_front()};
				}
			}

			void put_values(std::ostream& to, name_set const& from) const
			{
				for (auto const& it : map)
				{
					to << it.first << "= "<< it.second.string_view(from) << '\n';
				}
				to << std::flush;
			}

		private:
			std::unordered_map<std::string, variable_value_stack> map;
		};

		
		template<typename t>
		function_set fs_functs();
		template<typename t>
		std::string fs_name(name_set const& names);


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
					default_final_operation = name_checker{ std::move(*a) };
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
			std::optional<name_checker> default_final_operation;
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

			void attach(std::istream& input = std::cin, std::ostream& output = std::cout, option_set&& settings = option_set{})
			{
				function_set special_functions;
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
						perform(statement::val_type::make<function_call>(settings.default_final_operation->get_copy(), 1 ), result, variables, functions, garbage, errors);
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