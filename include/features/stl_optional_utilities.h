

#include"../../implementation/function_sets.h"

namespace expr
{
    namespace impl
    {

		template <typename t>
		struct type_operation_info<std::optional<t>> : public wrapper_type_operation_trait
		{

			static std::string print(std::optional<t> const& tar)
			{
				if (tar)
				{
					return "?" + type_operation_info<t const&>::print(*tar);
				}
				else
				{
					return "nullopt";
				}
			}



			template<typename name_generator = compiler_name_generator>
			static std::string type_name(name_generator instance)
			{
				return instance.template retrieve<t>() + "-optional";
			}



			static std::optional<std::optional<t>> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{
				if (start == stop)
				{
					//invalid input
					return std::optional<std::optional<t>>(std::nullopt);
				}

				static char const none[] = "nullopt";

				auto m = std::mismatch(start, stop, std::begin(none), std::end(none) - 1);

				if (m.second == std::end(none) - 1)
				{
					//valid input indicating an empty optional of t.
					start = m.first;
					return std::optional<std::optional<t>>(std::optional<t>(std::nullopt));
				}
				else if (*start == '?')
				{
					++start;
					std::optional<t>&& g = type_operation_info<t>::parse(start, stop);
					if (g)
					{
						return std::optional<std::optional<t>>(std::move(g));
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
		};
    }
}