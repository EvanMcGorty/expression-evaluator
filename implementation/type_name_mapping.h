#include <string>
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <cstdlib>

#include "type_wraps.h"
#include "call_typing.h"

#ifdef __GNUG__

#include <cxxabi.h>

namespace expr
{
	namespace impl
	{

		std::string demangle(const char* name) {

			int status = -4; // some arbitrary value to eliminate the compiler warning

			std::unique_ptr<char, void(*)(void*)> res{
				abi::__cxa_demangle(name, NULL, NULL, &status),
				std::free
			};

			return (status == 0) ? res.get() : name;
		}

	}
}

#else

namespace expr
{
	namespace impl
	{
		// does nothing if not g++
		std::string demangle(const char* name) {
			return name;
		}
	}
}

#endif

namespace expr
{
	namespace impl
	{

		struct name_set
		{
			std::unordered_map<std::type_index, std::string> data;

			~name_set() {}
		};



		static name_set* global_type_renames_object_pointer;
		
		inline void delete_global_type_renames_object()
		{
			delete global_type_renames_object_pointer;
		}


		name_set*& make_global_type_renames_object()
		{
			std::atexit(&delete_global_type_renames_object);
			global_type_renames_object_pointer = new name_set();
			return global_type_renames_object_pointer;
		}


		name_set& global_type_renames()
		{
			static name_set*& val = make_global_type_renames_object();
			return *val;
		}

		template<typename t>
		void rename(std::string&& new_name, name_set& names = global_type_renames())
		{

			static_assert(!(std::is_const_v<t>||std::is_reference_v<t>), "can only rename a raw type, references and const are not allowed");

			names.data[std::type_index{ typeid(t) }] = std::move(new_name);
			
		}
		
		template<typename t>
		std::string name_of(name_set const& names = global_type_renames())
		{
			static_assert(!type<t>::is_raw());

			using raw = typename type<t>::raw;

			if constexpr(type<t>::is_ref())
			{
				return name_of<val_wrap<raw>>(names) + "-ref";
			}
			else if constexpr(std::is_const_v<raw>)
			{
				return name_of<val_wrap<std::remove_const_t<raw>>>(names) + "-const";
			}
			else
			{
				auto g = names.data.find(std::type_index{ typeid(raw) });
				if (g == names.data.end())
				{
					if constexpr(type_wrap_info<raw>::is())
					{
						return name_of<post_return_t<typename type_wrap_info<raw>::wrapped>>(names) + type_wrap_info<raw>::suffix();
					}
					else
					{
						return demangle(typeid(raw).name());
					}
				}
				else
				{
					return std::string{ g->second };
				}
			}
		}

	}
}
