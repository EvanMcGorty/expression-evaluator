#pragma once
#include<string>
#include<memory>

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
		struct compiler_name_generator
		{
			template<typename t>
			std::string retrieve()
			{
				return demangle(typeid(t).name());
			}
		};
	}
}