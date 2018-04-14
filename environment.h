#pragma once
#include"function_evaluations.h"


namespace expr
{
	namespace impl
	{
		using held_callable = mu::virt<any_callable>;

		template<typename ret, typename...args>
		held_callable callable(std::function<ret(args...)>&& target)
		{
			return held_callable::make<callable_of<ret, args...>>(std::move(target));
		}

		template<typename ret, typename...args>
		held_callable callable(ret(*target)(args...))
		{
			return held_callable::make<callable_of<ret, args...>>((std::function<ret(args...)>{target}));
		}

		template<typename member_holders_type, typename ret, typename...args>
		held_callable callable(ret(member_holders_type::*target)(args...))
		{
			typedef member_holders_type& real_type;
			auto to_use = std::function<ret(real_type, args...)>{ target };
			return held_callable::make<callable_of<ret, real_type, args...>>(std::move(to_use));
		}

		template<typename member_holders_type, typename ret, typename...args>
		held_callable callable(ret(member_holders_type::*target)(args...) const)
		{
			typedef member_holders_type const& real_type;
			auto to_use = std::function<ret(real_type, args...)>{ target };
			return held_callable::make<callable_of<ret, real_type, args...>>(std::move(to_use));
		}

		template<typename member_holders_type, typename ret, typename...args>
		held_callable callable(ret(member_holders_type::*target)(args...) &&)
		{
			typedef member_holders_type&& real_type;
			auto to_use = std::function<ret(real_type, args...)>{ target };
			return held_callable::make<callable_of<ret, real_type, args...>>(std::move(to_use));
		}

		template<typename member_holders_type, typename ret, typename...args>
		held_callable callable(ret(member_holders_type::*target)(args...) const&&)
		{
			typedef member_holders_type const&& real_type;
			auto to_use = std::function<ret(real_type, args...)>{ target };
			return held_callable::make<callable_of<ret, real_type, args...>>(std::move(to_use));
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

			function_set& add(held_callable&& f, std::string&& n)
			{
				assert(name_checker::is_valid(n));
				emplace_to_map(std::make_pair(std::move(n), std::move(f)));
				return *this;
			}

			function_set& add(held_callable&& f, std::string const& n)
			{
				assert(name_checker::is_valid(n));
				emplace_to_map(std::make_pair(n, std::move(f)));
				return *this;
			}
			
			function_set& add(held_callable&& f, char const* n)
			{
				assert(name_checker::is_valid(n));
				emplace_to_map(std::make_pair(std::string{n}, std::move(f)));
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
					emplace_to_map(std::make_pair(std::string{ n + "." + cur.first }, std::move(cur.second)));
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
					emplace_to_map(std::make_pair(std::string{ std::string{n} +"." + cur.first }, std::move(cur.second)));
				}
				return *this;
			}

			function_set& merge(function_set&& w)
			{
				for (auto it = w.map.begin(); it != w.map.end(); ++it)
				{
					emplace_to_map(std::move(*it));
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

		private:

			void emplace_to_map(std::pair<std::string, held_callable>&& a)
			{
				map.emplace(std::move(a));
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

		private:
			std::unordered_map<std::string, variable_value_stack> map;
		};

		
		template<typename t>
		function_set fs_functs();
		template<typename t>
		function_set fs_name();

		class environment
		{
			friend void perform(statement&& todo, stack& loc, environment& env);
			friend void perform_all(executable&& tar, stack& loc, environment& env);
		public:



			void run(executable&& a);

			void run(elem&& a);

			void attach(std::istream& input, std::ostream& output)
			{
				while (true)
				{
					output << "\\\\\\\n" << std::flush;
					elem n;
					input >> n;
					output << "///\n" << std::flush;
					run(std::move(n));
				}
			}

			template<typename string_convertible>
			environment& mbind(string_convertible&& name, std::function<value_holder(std::vector<stack_elem>&)>&& target)
			{
				functions.add(manual(std::move(target)), std::forward<string_convertible>(name));
				return *this;
			}

			template<typename string_convertible>
			environment& mbind(string_convertible&& name, value_holder(*target)(std::vector<stack_elem>&))
			{
				functions.add(manual(target), std::forward<string_convertible>(name));
				return *this;
			}

			template<typename string_convertible>
			environment& fbind(string_convertible&& name, held_callable&& target)
			{
				functions.add(std::move(target),std::forward<string_convertible>(name));
				return *this;
			}
			
			template<typename string_convertible, typename ret, typename...args>
			environment& fbind(string_convertible&& name, std::function<ret(args...)>&& target)
			{
				functions.add(callable(std::move(target)),std::forward<string_convertible>(name));
				return *this;
			}

			template<typename string_convertible, typename ret, typename...args>
			environment& fbind(string_convertible&& name, ret(*target)(args...))
			{
				functions.add(callable(target), std::forward<string_convertible>(name));
				return *this;
			}

			template<typename string_convertible, typename member_holders_type, typename ret, typename...args>
			environment& fbind(string_convertible&& name, ret(member_holders_type::*target)(args...))
			{
				functions.add(callable(target), std::forward<string_convertible>(name));
				return *this;
			}
			template<typename string_convertible, typename member_holders_type, typename ret, typename...args>
			environment& fbind(string_convertible&& name, ret(member_holders_type::*target)(args...) const)
			{
				functions.add(callable(target), std::forward<string_convertible>(name));
				return *this;
			}
			template<typename string_convertible, typename member_holders_type, typename ret, typename...args>
			environment& fbind(string_convertible&& name, ret(member_holders_type::*target)(args...) &&)
			{
				functions.add(callable(target), std::forward<string_convertible>(name));
				return *this;
			}
			template<typename string_convertible, typename member_holders_type, typename ret, typename...args>
			environment& fbind(string_convertible&& name, ret(member_holders_type::*target)(args...) const&&)
			{
				functions.add(callable(target), std::forward<string_convertible>(name));
				return *this;
			}

			template<typename t, typename string_convertible>
			environment& vbind(string_convertible&& name, t& a)
			{
				functions.add(callable(std::function<t()>{[ref = a]() {return ref; }}), std::forward<string_convertible>(name));
				return *this;
			}

			template<typename t = void, typename string_convertible>
			environment& sbind(string_convertible&& name = fs_name<t>(), function_set&& set = fs_functs<t>())
			{
				functions.use(std::move(set), std::forward<string_convertible>(name));
				return *this;
			}

			
			held_callable garbage_getter()
			{
				variable_value_stack* g = &garbage;
				return manual(std::function<value_holder(std::vector<stack_elem>&)>{
					[g = g](std::vector<stack_elem>& a) -> value_holder
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
		private:
			function_set functions;

			variable_set variables;
			variable_value_stack garbage;
		};
	}
}