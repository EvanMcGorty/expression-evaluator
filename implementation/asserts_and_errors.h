#pragma once
#include<exception>

namespace expr
{
	namespace impl
	{
		using expression_evaluator_internal_logic_error = std::logic_error;

		using expression_evaluator_usage_logic_error = std::logic_error;

#ifdef _DEBUG
		template<typename InvokesToBool>
		void assert_with_generic_logic_error(InvokesToBool a)
		{
			if (!a())
			{
				throw expression_evaluator_internal_logic_error("the expression evaluator has an internal logic error");
			}
		}
		template<typename InvokesToBool>
		void assert_with_invalid_method_usage(InvokesToBool a)
		{
			if (!a())
			{
				throw expression_evaluator_usage_logic_error("usage of invalid method");
			}
		}
		template<typename InvokesToBool>
		void assert_with_invalid_name_usage(InvokesToBool a)
		{
			if (!a())
			{
				throw expression_evaluator_usage_logic_error("usage of invalid name");
			}
		}
#else
		template<typename InvokesToBool>
		void assert_with_generic_logic_error(InvokesToBool) {}
		template<typename InvokesToBool>
		void assert_with_invalid_method_usage(InvokesToBool) {}
		template<typename InvokesToBool>
		void assert_with_invalid_name_usage(InvokesToBool) {}
#endif

	}
}