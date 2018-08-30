#include"../../../implementation/objects_and_calls/evaluator_basics/type_name_mapping.h"


namespace expr
{
	namespace impl
	{

		template<>
		struct global_type_info_maker<void>
		{
			static type_info_set*& make_global_type_info_object()
			{
				std::atexit(&delete_global_type_info_object);
				global_type_info_object_pointer = new type_info_set();
				return global_type_info_object_pointer;
			}

			template<typename t>
			void declare_with_name(std::string&& new_name)
			{
				declare_with_name<t>(std::move(new_name),global_type_info());
			}

			template<typename t>
			void declare()
			{
				declare_with_name<t>(global_type_info());
			}

			template<typename t>
			std::string name_of()
			{
				return name_of<t>(global_type_info());
			}
		};

	}

	using impl::global_type_info;
}