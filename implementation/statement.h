#pragma once
#include"tree_structure.h"

namespace expr
{
	namespace impl
	{

		class operation
		{
		public:

			operation() = default;
			operation(operation const&) = default;
			operation(operation&&) = default;

			virtual ~operation()
			{ }

			virtual bool is_literal_push() const
			{
				return false;
			}

			virtual bool is_variable_push() const
			{
				return false;
			}


			virtual bool is_function_call() const
			{
				return false;
			}

			virtual std::string view() const = 0;

		};


		class literal_push : public operation
		{
		public:

			literal_push(literal_value&& a) :
				val(std::move(a))
			{ }

			literal val;

			bool is_literal_push() const final override
			{
				return true;
			}

			std::string view() const override
			{
				return std::string("literal_push{") + val.make_string() + "}";
			}
		};

		class variable_push : public operation
		{
		public:

			variable_push(variable_value&& a) :
				var(std::move(a))
			{ }

			variable var;

			bool is_variable_push() const final override
			{
				return true;
			}

			std::string view() const override
			{
				return std::string("variable_push{") + var.make_string() + "}";
			}
		};



		class function_call : public operation
		{
		public:

			function_call(function_value const& a)
			{
				name = a.fn_name;
				arg_count = a.arguments.size();
			}

			function_call(std::string fn_name, size_t size)
			{
				name = fn_name;
				arg_count = size;
			}

			std::string name;
			size_t arg_count;

			bool is_function_call() const final override
			{
				return true;
			}

			std::string view() const override
			{
				return std::string("function_call{") + name + ", " + std::to_string(arg_count) + "}";
			}

		};

		class statement
		{
		public:

			using val_type = mu::algebraic<operation, literal_push, variable_push, function_call>;

			statement(val_type&& a) :
				val(std::move(a))
			{ }

			val_type val;
		};

		class executable
		{
		public:
			std::vector<statement> statements;
		};

		inline void elem::make_executable(executable& result_location) const
		{
			std::vector<statement>& a = result_location.statements;
			if (val.is_nullval())
			{
				a.push_back(statement::val_type::make_nullval());
			}
			else if (val->is_literal())
			{
				a.push_back(statement::val_type::make<literal_push>(literal_value{ val.downcast_get<literal>()->data }));
			}
			else if (val->is_variable())
			{
				a.push_back(statement::val_type::make<variable_push>(variable_value{ val.downcast_get<variable>()->data }));
			}
			else if (val->is_function())
			{
				function const* f = val.downcast_get<function>();
				for (auto const& it : f->data.arguments)
				{
					it.make_executable(result_location);
				}
				a.push_back(statement::val_type::make<function_call>(val.downcast_get<function>()->data ));
			}
		}

		inline void elem::into_executable(executable& result_location) &&
		{
			std::vector<statement>& a = result_location.statements;
			if (val.is_nullval())
			{
				a.push_back(statement::val_type::make_nullval());
			}
			else if (val->is_literal())
			{
				a.push_back(statement::val_type::make<literal_push>(literal_value{ std::move(val.downcast_get<literal>()->data) }));
			}
			else if (val->is_variable())
			{
				a.push_back(statement::val_type::make<variable_push>(variable_value{ std::move(val.downcast_get<variable>()->data) }));
			}
			else if (val->is_function())
			{
				function* f = val.downcast_get<function>();
				for (auto& it : f->data.arguments)
				{
					std::move(it).into_executable(result_location);
				}
				a.push_back(statement::val_type::make<function_call>(function_value{ std::move(val.downcast_get<function>()->data) }));
			}
		}
	}
}
