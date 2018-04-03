#pragma once
#include"function_evaluations.h"

namespace expressions
{

	class function_set
	{
	public:

		template<typename ret, typename...args>
		function_set& add_auto(std::function<ret(args...)>&& target, std::string name)
		{
			map.emplace(name, std::move(mu::virt<any_callable>::make<callable_of<ret, args...>>(std::move(target))));
			return *this;
		}

		template<typename ret, typename...args>
		function_set& add_auto(ret(*target)(args...), std::string name)
		{
			return add_auto(std::function<ret(args...)>{target}, std::move(name));
		}

		function_set& add_manual(std::function<value_holder(std::vector<stack_elem>&&)>&& target, std::string name)
		{
			map.emplace(name, std::move(mu::virt<any_callable>::make<manual_callable>(std::move(target))));
			return *this;
		}

		function_set& add_manual(value_holder(*target)(std::vector<stack_elem>&&), std::string name)
		{
			return add_manual(std::function<value_holder(std::vector<stack_elem>&&)>{target},name);
		}



		std::optional<mu::virt<any_callable>*> get(std::string const& a)
		{
			auto g = map.find(a);
			if (g == map.end())
			{
				return std::nullopt;
			}
			else
			{
				return std::optional<mu::virt<any_callable>*>{&g->second};
			}
		}

	private:
		std::unordered_map<std::string, mu::virt<any_callable>> map;
	};


	class variable_value_stack
	{
	public:

		value_holder* push_front(value_holder&& a)
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



	class environment
	{
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

		function_set functions;

		variable_set variables;
		variable_value_stack garbage;
	};
}