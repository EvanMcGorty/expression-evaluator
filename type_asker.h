#pragma once
#include"literal_conversions.h"

#include<typeinfo>
#include<any>

namespace expressions
{

	class any_type_ask
	{
	public:
		virtual void parse(std::string const& a) = 0;

		virtual std::type_info const& get_type() const = 0;

		virtual ~any_type_ask()
		{

		}
	};

	template<typename t>
	class type_ask_of : public any_type_ask
	{
	public:

		template<bool b>
		struct possible;

		template<> struct possible<true>
		{
			possible(std::remove_const_t<typename std::remove_pointer_t<t>>&& a)
			{
				val = std::move(a);
			}

			std::remove_const_t<typename std::remove_pointer_t<t>> val;
		};

		template<> struct possible<false>
		{

		};

		void parse(std::string const& a) override
		{
			if constexpr(std::is_pointer<t>::value)
			{
				typedef std::remove_const_t<typename std::remove_pointer_t<t>> holdable;
				std::optional<holdable> temp = convert<holdable>(a);
				if (temp)
				{
					pointed_to_value = possible<true>{ std::move(*temp) };
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
		std::optional<possible<std::is_pointer_v<t>>> pointed_to_value;
	};

}