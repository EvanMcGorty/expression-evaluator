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
				auto temp = convert<typename std::remove_const<typename std::remove_pointer<t>::type>::type>(a);
				if (temp)
				{
					b = std::move(*temp); gotten = std::optional<t>{ std::any_cast<typename std::remove_const<typename std::remove_pointer<t>::type>::type>(&b) };
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