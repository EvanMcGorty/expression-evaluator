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

			std::string string_view() const
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
						ret.append(it->string_view());
					}
					ret.push_back(',');
				}
				*ret.rbegin() = ']';
				return ret;
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

			void put_values(std::ostream& to) const
			{
				for (auto const& it : map)
				{
					to << it.first << "= "<< it.second.string_view() << '\n';
				}
				to << std::flush;
			}

		private:
			std::unordered_map<std::string, variable_value_stack> map;
		};

		
		template<typename t>
		function_set fs_functs();
		template<typename t>
		std::string fs_name();

		class environment
		{
			friend void perform(statement&& todo, stack& loc, environment& env, std::ostream& errors);
			friend void perform_all(executable&& tar, stack& loc, environment& env, std::ostream& errors);
		public:



			void run(executable&& a, std::ostream& errors);

			void run(elem&& a, std::ostream& errors);

			void attach(std::istream& input, std::ostream& output)
			{
				while (true)
				{
					output << "\\\\\\\n" << std::flush;
					elem n;
					input >> n;
					if (settings.whether_to_reprint_input_as_parsed)
					{
						output << "|||\n" << n.str() << "\n";
					}
					if (settings.default_final_operation)
					{
						n = elem::make(function_value{ settings.default_final_operation->get_copy(), std::vector<elem>{ std::move(n) } });
					}
					output << "///" << std::endl;
					std::stringstream errors;
					run(std::move(n),errors);
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
			environment& rebind(string_convertible&& name, held_callable&& target)
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
			environment& sbind(string_convertible&& name = fs_name<t>(), function_set&& set = fs_functs<t>())
			{
				functions.use(std::move(set), std::forward<string_convertible>(name));
				return *this;
			}


			held_callable value_printer(std::ostream& to)
			{
				return manual(std::function<value_holder(std::vector<stack_elem>&)>{[to = &to](std::vector<stack_elem>& a) -> value_holder
					{
						if (a.size() == 1 && !a[0].is_nullval())
						{
							*to << a[0]->string_view() << std::endl;
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
								return std::move(*f);
							}
						}
						return value_holder::make_nullval();
					}
				});
			}

			held_callable functions_printer(std::ostream& to)
			{
				return callable(std::function<void()>{
					[to = &to,fs =&functions]() -> void
					{
						for (auto const& it : fs->view())
						{
							*to << it.first;
							it.second->put_type(*to);
							*to << '\n' << std::flush;
						}
					}
				});
			}

			held_callable variables_printer(std::ostream& to)
			{
				return callable(std::function<void()>{
					[to = &to, vs = &variables]() -> void
					{
						vs->put_values(*to);
					}
				});
			}

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

			option_set settings;

		private:
			function_set functions;

			variable_set variables;
			variable_value_stack garbage;
		};
	}
}