#pragma once
#include "../../../implementation/objects_and_calls/evaluator_basics/type_name_mapping.h"


namespace expr
{
	namespace impl
	{
		type_info_set* global_type_info();

		template<>
		struct global_type_info_enabler<void>
		{
			static type_info_set* get()
			{
				return global_type_info();
			}
		};

		static type_info_set* global_type_info_object_pointer;

		inline void delete_global_type_info_object()
		{
			delete global_type_info_object_pointer;
		}


		static type_info_set*& make_global_type_info_object()
		{
			std::atexit(&delete_global_type_info_object);
			global_type_info_object_pointer = new type_info_set();
			return global_type_info_object_pointer;
		}

		type_info_set* global_type_info()
		{
			static type_info_set*& val = make_global_type_info_object();
			return val;
		}

		template <typename t>
		void declare_with_name(std::string&& new_name)
		{
			declare_with_name<t>(std::move(new_name), global_type_info());
		}

		template <typename...t>
		void declare()
		{
			declare<t...>(global_type_info());
		}

	}

	using impl::global_type_info;

}