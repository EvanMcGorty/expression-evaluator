#include <string>
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <typeindex>

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
		std::unordered_map<std::type_index, std::string> renamed_types;

		template<typename type>
		void rename(std::string&& new_name)
		{
			static_assert(!(std::is_const_v<type>||std::is_pointer_v<type>||std::is_reference_v<type>), "can only rename a raw class/struct/union");

			renamed_types[std::type_index{ typeid(type) }] = std::move(new_name);
			
		}
		
		//these wierd function signatures let someone do name_of<compile_time_known_type>() or name_of(type_info_for_runtime_known_type)
		template<typename type>
		std::string name_of()
		{
			if constexpr(std::is_reference_v<type>)
			{
				return name_of<std::remove_reference_t<type>*>();
			}
			else if constexpr(std::is_const_v<type>)
			{
				return name_of<std::remove_const_t<type>>() + "-const";
			}

			auto g = renamed_types.find(std::type_index{ typeid(type) });
			if (g == renamed_types.end())
			{
				if constexpr(std::is_pointer_v<type>)
				{
					return name_of<std::remove_pointer_t<type>>() + "-ref";
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