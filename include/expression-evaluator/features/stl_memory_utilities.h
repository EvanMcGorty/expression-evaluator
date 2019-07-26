#pragma once
#include"../../../implementation/function_sets.h"

namespace expr
{
    namespace impl
    {
        

		template <typename t>
		struct type_operation_info<std::unique_ptr<t>> : public wrapper_type_operation_trait
		{

			template<typename name_generator = compiler_name_generator>
			static std::string type_name(name_generator instance)
			{
				return instance.template retrieve<t>() + "-unique_ptr";
			}

			static std::optional<std::unique_ptr<t>> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{
				if (start == stop)
				{
					//invalid input
					return std::optional<std::unique_ptr<t>>(nullptr);
				}

				static char const none[] = "nullptr";

				auto m = std::mismatch(start, stop, std::begin(none), std::end(none) - 1);

				if (m.second == std::end(none) - 1)
				{
					start = m.first;
					return std::optional<std::unique_ptr<t>>(std::unique_ptr<t>(nullptr));
				}
				else if (*start == '&')
				{
					++start;
					std::optional<t>&& g = type_operation_info<t>::parse(start, stop);
					if (g)
					{
						return std::optional<std::unique_ptr<t>>(std::make_unique<t>(std::move(*g)));
					}
					else
					{
						return std::nullopt;
					}
				}
				else
				{
					return std::nullopt;
				}
			}

			static std::string print(std::unique_ptr<t> const& tar)
			{
				if (tar)
				{
					return "&" + type_operation_info<t>::print(*tar);
				}
				else
				{
					return "nullptr";
				}
			}

		};

		template <typename t>
		struct type_operation_info<std::shared_ptr<t>> : public wrapper_type_operation_trait
		{

			template<typename name_generator = compiler_name_generator>
			static std::string type_name(name_generator instance)
			{
				return instance.template retrieve<t>() + "-shared_ptr";
			}

			static std::optional<std::shared_ptr<t>> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{
				if (start == stop)
				{
					//invalid input
					return std::optional<std::shared_ptr<t>>(nullptr);
				}

				static char const none[] = "nullptr";

				auto m = std::mismatch(start, stop, std::begin(none), std::end(none) - 1);

				if (m.second == std::end(none) - 1)
				{
					start = m.first;
					return std::optional<std::shared_ptr<t>>(std::shared_ptr<t>(nullptr));
				}
				else if (*start == '&')
				{
					++start;
					std::optional<t>&& g = type_operation_info<t>::parse(start, stop);
					if (g)
					{
						return std::optional<std::shared_ptr<t>>(std::make_unique<t>(std::move(*g)));
					}
					else
					{
						return std::nullopt;
					}
				}
				else
				{
					return std::nullopt;
				}
			}

			static std::string print(std::shared_ptr<t> const& tar)
			{
				if (tar)
				{
					return "&" + type_operation_info<t>::print(*tar);
				}
				else
				{
					return "nullptr";
				}
			}
		};
    }
}