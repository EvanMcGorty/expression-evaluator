#include"../../../implementation/objects_and_calls/evaluator_basics/type_operation_info.h"

namespace expr
{
    namespace impl
    {
		template<typename t>
		struct wrapper_type_operation_info<t,std::enable_if_t<std::is_base_of_v<wrapper_type_operation_trait, type_operation_info<t>>>>
		{
			static constexpr bool is_wrapper()
			{
				return true;
			}

			using wrapped = decltype(*std::declval<t>());

			static bool can_unwrap(t&& a)
			{
				if (std::forward<t>(a))
				{
					return true;
				}
				else
				{
					return false;
				}
			}

			static wrapped do_unwrap(t&& a)
			{
				return *std::forward<t>(a);
			}
		};
    }
}