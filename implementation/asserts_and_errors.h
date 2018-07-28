#pragma once
#include<exception>

namespace expr
{
	namespace impl
	{
		using expression_evaluator_internal_logic_error = std::logic_error;

		using expression_evaluator_usage_logic_error = std::logic_error;

#ifdef _DEBUG
		void assert_with_generic_logic_error(bool a)
		{
			if (!a)
			{
				throw expression_evaluator_internal_logic_error("the expression evaluator has an internal logic error");
			}
		}
		void assert_with_invalid_method_usage(bool a)
		{
			if (!a)
			{
				throw expression_evaluator_usage_logic_error("usage of invalid method");
			}
		}
		void assert_with_invalid_name_usage(bool a)
		{
			if (!a)
			{
				throw expression_evaluator_usage_logic_error("usage of invalid name");
			}
		}
#else
		void assert_with_generic_logic_error(bool) {}
		void assert_with_invalid_method_usage(bool) {}
		void assert_with_invalid_name_usage(bool) {}
#endif

	}
}