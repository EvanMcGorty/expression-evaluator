#pragma once
#include"statement_evaluations.h"

namespace expressions
{

	struct core
	{

		static value_holder swap(std::vector<stack_elem>& a)
		{
			if (a.size() == 2 && a[0]->has_value() && a[1]->has_value())
			{
				auto o0 = get_value(a[0]);
				auto o1 = get_value(a[1]);
				set_value(a[0], std::move(o1));
				set_value(a[1], std::move(o0));
			}
			return value_holder::make_nullval();
		}

		static value_holder first(std::vector<stack_elem>& a)
		{
			if (a.size() == 0)
			{
				return value_holder::make_nullval();
			}
			else if((*a.begin())->has_value())
			{
				return get_value(*a.begin());
			}
			else
			{
				return value_holder::make_nullval();
			}
		}


		static value_holder last(std::vector<stack_elem>& a)
		{
			if (a.size() == 0)
			{
				return value_holder::make_nullval();
			}
			else if ((*a.rbegin())->has_value())
			{
				return get_value(*a.rbegin());
			}
			else
			{
				return value_holder::make_nullval();
			}
		}

		static value_holder drop(std::vector<stack_elem>& a)
		{
			for (auto& it : a)
			{
				if (!it.is_nullval())
				{
					if (it->has_value())
					{
						get_value(it);
					}
					else
					{
						continue;
					}
				}
			}
			return value_holder::make_nullval();
		}
	};

}