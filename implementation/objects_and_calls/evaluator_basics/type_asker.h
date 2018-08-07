#pragma once

#include<typeinfo>
#include<optional>
#include"call_typing.h"

namespace expr
{
	namespace impl
	{

		class any_type_ask
		{
		public:
			virtual void parse(std::string const& a) = 0;

			virtual void lazy_parse(std::string const& a) = 0;

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
			possible(std::remove_const_t<typename type<t>::raw> &&a) :
				val(std::move(a))
			{}

			std::remove_const_t<typename type<t>::raw> val;
		};

		template<typename t>
		struct possible<false, t>
		{
		};
		
		template<typename t>
		struct type_operation_info;

		template<typename t>
		class type_ask_of : public any_type_ask
		{
		public:



			void parse(std::string const& a) override
			{
				if constexpr(type<t>::is_ref())
				{
					typedef std::remove_const_t<typename type<t>::raw> holdable;
					auto it = a.cbegin();
					std::optional<holdable> temp = type_operation_info<typename type<t>::raw>::parse(it,a.cend());
					if (temp)
					{
						pointed_to_value.emplace(possible<true, t>{ std::move(*temp) });
						gotten.emplace(pointed_to_value->val);
					}
				}
				else
				{
					auto it = a.cbegin();
					auto temp = type_operation_info<typename type<t>::raw>::parse(it,a.cend());
					if (temp)
					{
						gotten.emplace(std::move(*temp));
					}
				}
			}

			void lazy_parse(std::string const& a) override
			{
				if constexpr(type<t>::is_ref())
				{
					
				}
				else
				{
					auto it = a.cbegin();
					auto temp = type_operation_info<typename type<t>::raw>::parse(it, a.cend());
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
			std::optional<possible<type<t>::is_ref(), t>> pointed_to_value;
		};

	}
}
