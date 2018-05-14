#include <string>
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <typeindex>

#include "type_qualities.h"

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
		};

		name_set global_type_renames;

		template<typename type>
		void rename(std::string&& new_name, name_set& names = global_type_renames)
		{
			static_assert(!(std::is_const_v<type>||pointer<type>::is()||std::is_reference_v<type>), "can only rename a raw class/struct/union");

			names.data[std::type_index{ typeid(type) }] = std::move(new_name);
			
		}
		
		template<typename type>
		std::string name_of(name_set const& names = global_type_renames)
		{
			if constexpr(std::is_reference_v<type>)
			{
				return name_of<std::remove_reference_t<type>*>(names);
			}
			else if constexpr(std::is_const_v<type>)
			{
				return name_of<std::remove_const_t<type>>(names) + "-const";
			}

			auto g = names.data.find(std::type_index{ typeid(type) });
			if (g == names.data.end())
			{
				if constexpr(pointer<type>::is())
				{
					return name_of<typename pointer<type>::deref>(names) + pointer<type>::suffix();
				}
				else
				{
					return demangle(typeid(type).name());
				}
			}
			else
			{
				return std::string{ g->second };
			}
		}

	}
}