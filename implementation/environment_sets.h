#pragma once
#include<deque>
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
			std::deque<value_holder> values;
			//deque instead of vector because references must not be invalidated after a push.
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

    }
}