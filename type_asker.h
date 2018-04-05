#pragma once
#include"literal_conversions.h"

#include<typeinfo>
#include<any>

namespace expressions
{

	class any_type_ask
	{
	public:
		virtual void parse(std::string const& a, std::any& b) = 0;

		virtual std::type_info const& get_type() const = 0;

		virtual ~any_type_ask()
		{

		}
	};

	template<typename t>
	class type_ask_of : public any_type_ask
	{
	public:

		void parse(std::string const& a, std::any& b) override
		{
			if constexpr(std::is_pointer<t>::value)
			{
				typedef typename std::remove_const<typename std::remove_pointer<t>::type>::type holdable;
				std::optional<holdable> temp = convert<holdable>(a);
				if (temp)
				{
					gotten = std::optional<t>{ &b.emplace<holdable>(std::move(*temp)) };
				}
			}
			else
			{
				gotten = convert<t>(a);
			}
		}

		std::type_info const& get_type() const override
		{
			return typeid(t);
		}

		std::optional<t> gotten;
	};

}