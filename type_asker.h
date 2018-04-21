#pragma once
#include"literal_conversions.h"

#include<typeinfo>
#include<any>

namespace expr
{
	namespace impl
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

		template <bool b, typename t>
		struct possible;

		template<typename t>
		struct possible<true, t>
		{
			possible(std::remove_const_t<typename std::remove_pointer_t<t>> &&a) :
				val(std::move(a))
			{}

			std::remove_const_t<typename std::remove_pointer_t<t>> val;
		};

		template<typename t>
		struct possible<false, t>
		{
		};

		template<typename t>
		class type_ask_of : public any_type_ask
		{
		public:



			void parse(std::string const& a) override
			{
				if constexpr(std::is_pointer_v<t>)
				{
					typedef std::remove_const_t<typename std::remove_pointer_t<t>> holdable;
					std::optional<holdable> temp = converter<holdable>::on(a);
					if (temp)
					{
						pointed_to_value.emplace(possible<true, t>{ std::move(*temp) });
					}
				}
				else
				{
					auto temp = converter<std::remove_const_t<t>>::on(a);
					if (temp)
					{
						gotten.emplace(std::move(*temp));
					}
				}
			}

			std::type_info const& get_type() const override
			{
				return typeid(t);
			}

			std::optional<std::remove_const_t<t>> gotten;
			std::optional<possible<std::is_pointer_v<t>, t>> pointed_to_value;
		};

	}
}