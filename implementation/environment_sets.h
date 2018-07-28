#pragma once
#include<deque>
#include"function_evaluations.h"


namespace expr
{
	namespace impl
	{

		template<typename t>
		t* pick(t* a)
		{
			return a;
		}

		template<typename ret, typename...args>
		held_callable sfn(std::function<ret(args...)>&& target)
		{
			return held_callable::make<smart_callable<ret, args...>>(make_storable_call(std::move(target)));
		}

		template<typename ret, typename...args>
		held_callable sfn(ret(*target)(args...))
		{
			return sfn( std::function<ret(args...)>{target} );
		}

		template<typename member_holders_type, typename ret, typename...args>
		held_callable sfn(ret(member_holders_type::*target)(args...))
		{
			typedef member_holders_type& real_type;
			return sfn( std::function<ret(real_type, args...)>{ target } );
		}

		template<typename member_holders_type, typename ret, typename...args>
		held_callable sfn(ret(member_holders_type::*target)(args...) const)
		{
			typedef member_holders_type const& real_type;
			return sfn( std::function<ret(real_type, args...)>{ target } );
		}

		template<typename member_holders_type, typename ret, typename...args>
		held_callable sfn(ret(member_holders_type::*target)(args...) &&)
		{
			typedef member_holders_type&& real_type;
			return  sfn(std::function<ret(real_type, args...)>{ target });
		}

		template<typename member_holders_type, typename ret, typename...args>
		held_callable sfn(ret(member_holders_type::*target)(args...) const&&)
		{
			typedef member_holders_type const&& real_type;
			return sfn(std::function<ret(real_type, args...)>{ target });
		}

		template<typename member_holders_type, typename members_type>
		held_callable mbr(members_type member_holders_type::*mbr)
		{
			held_callable constant_version = sfn(std::function<members_type const&(member_holders_type const&)>{[pmember = mbr](member_holders_type const& a) -> members_type const&
			{
				return a.*pmember;
			}
			});

			held_callable mutable_version = sfn(std::function<members_type&(member_holders_type&)>{[pmember = mbr](member_holders_type& a) -> members_type&
			{
				return a.*pmember;
			}
			});
			return std::move(*mutable_version).add_layer(std::move(constant_version));
		}

		template<typename t>
		held_callable copier(t&& a)
		{
			return sfn(std::function<t()>{[val = std::move(a)]()->t {return t{ val }; }});
		}
		
		template<typename t>
		held_callable refto(t& a)
		{
			return sfn(std::function<t&()>{[val = &a]()->t& {return *val; }});
		}

		held_callable mfn(std::function<object_holder(std::vector<stack_elem>&)>&& target)
		{
			return held_callable::make<manual_callable>(std::move(target));
		}

		held_callable mfn(object_holder(*target)(std::vector<stack_elem>&))
		{
			return held_callable::make<manual_callable>(std::function<object_holder(std::vector<stack_elem>&)>{target});
		}

		template<typename into, typename from>
		held_callable cfn()
		{
			return sfn(std::function<into(from&&)>{
				[](from&& a) -> into
				{
					return static_cast<into>(std::forward<from>(a));
				}
			});
		}

		class function_insertion;

		class function_set;

		template<typename t>
		std::string fs_name(const name_set &names);

		template<typename t>
		function_set fs_functs();

		class function_set
		{
		public:

			function_insertion operator<<(std::string name);

			function_set& remove(std::string const& b)
			{
				map.erase(b);
				return *this;
			}

			template<typename string_convertible>
			function_set& add(held_callable&& f, string_convertible&& n)
			{
				assert_with_invalid_name_usage(name_checker::is_valid(std::string{ n }));
				add_to_map(std::make_pair(std::string{ n }, std::move(f)));
				return *this;
			}


			template<typename t = void, typename string_convertible = std::string>
			function_set& use(string_convertible&& n = fs_name<t>(global_type_renames()), function_set&& set = fs_functs<t>())
			{
				std::string name{ n };
				if (name == "")
				{
					for (auto it = set.map.begin(); it != set.map.end(); ++it)
					{
						add_to_map(std::move(*it));
					}
					return *this;
				}
				assert_with_invalid_name_usage(name_checker::is_valid(std::string{ name }));
				for (auto it = set.map.begin(); it != set.map.end(); ++it)
				{
					auto cur = std::move(*it);
					add_to_map(std::make_pair(std::string{ name + "." + cur.first }, std::move(cur.second)));
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
					return ret;
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
					g->second = std::move(*g->second).add_layer(std::move(a.second));
				}
				else
				{
					map.emplace(std::move(a.first), std::move(a.second));
				}
			}

			std::unordered_map<std::string, held_callable> map;
		};

		class function_insertion
		{
		public:

			function_insertion(std::string&& a, function_set& b) :
				to(b),
				at(std::move(a))
			{}

			function_insertion operator<<(function_set&& to_import)
			{
				to.use(at, std::move(to_import));
				return function_insertion{ std::move(at), to };
			}

			function_insertion operator<<(held_callable&& to_insert) &&
			{
				to.add(std::move(to_insert), at);
				return function_insertion{ std::move(at), to };
			}

			function_insertion operator<<(std::string name) &&
			{
				return function_insertion{std::move(name), to};
			}

		private:
			function_set& to;
			std::string at;
		};

		inline function_insertion function_set::operator<<(std::string name)
		{
			return function_insertion{ std::move(name),*this };
		}


		class variable_value_stack
		{
		public:

			object_holder * push_front(object_holder&& a)
			{
				values.emplace_back(std::move(a));
				return &*values.rbegin();
			}

			std::optional<object_holder*> get_front()
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

			std::optional<object_holder> take_front()
			{
				if (values.size() == 0)
				{
					return std::nullopt;
				}
				else
				{
					auto ret = std::move(*values.rbegin());
					values.pop_back();
					return std::optional<object_holder>{std::move(ret)};
				}
			}

			std::string string_view(name_set const& from)
			{
				if (values.size() == 0)
				{
					return "[]";
				}
				std::string ret{ "[" };
				for (auto& it : values)
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

			void clean_to_front(stack_elem& from, std::ostream& info)
			{
				if (!from.is_nullval() && from->is_object())
				{
					auto to_push = std::move(from).downcast<any_object>();
					if (!to_push->can_trivially_destruct())
					{
						info << to_push->string_view(global_type_renames()) << " cleaned to stack front\n";
						push_front(std::move(to_push));
					}
				}
			}

			void clean_all_to_front(stack& from, size_t count, std::ostream& info)
			{
				assert_with_generic_logic_error(count <= from.stuff.size());

				for (size_t i = 0; i != count; ++i)
				{
					clean_to_front(*from.stuff.rbegin(), info);
					from.stuff.pop_back();
				}
			}

		private:
			std::deque<object_holder> values;
			//deque instead of vector because references must not be invalidated after a push.
		};

		class variable_set
		{
			friend class environment;
		public:


			object_holder* push_var(std::string&& a)
			{
				return map[a].push_front(object_holder::make_nullval());
			}

			std::optional<object_holder*> get_var(std::string const& a)
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

			std::optional<object_holder> take_var(std::string const& a)
			{
				auto it = map.find(a);
				if (it == map.end())
				{
					return std::nullopt;
				}
				else
				{
					return std::optional<object_holder>{it->second.take_front()};
				}
			}

			void put_values(std::ostream& to, name_set const& from)
			{
				for (auto& it : map)
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
