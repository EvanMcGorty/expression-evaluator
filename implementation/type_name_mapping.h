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
		struct predeclared_object_result;

		class type_operations
		{
			virtual predeclared_object_result make_from_string(std::string::const_iterator& start, std::string::const_iterator stop) const = 0;
		};

		template<typename t>
		class type_operations_for : public type_operations
		{
			predeclared_object_result make_from_string(std::string::const_iterator& start, std::string::const_iterator stop) const override;
		};


		template<typename t>
		constexpr type_operations_for<t> global_type_operation_adresses{};

		struct type_info_set
		{
			std::unordered_map<std::type_index, std::string> names;
			std::unordered_map<std::string, type_operations const*> operations;

			~type_info_set() {}
		};



		static type_info_set* global_type_info_object_pointer;
		
		inline void delete_global_type_info_object()
		{
			delete global_type_info_object_pointer;
		}


		type_info_set*& make_global_type_info_object()
		{
			std::atexit(&delete_global_type_info_object);
			global_type_info_object_pointer = new type_info_set();
			return global_type_info_object_pointer;
		}


		type_info_set& global_type_info()
		{
			static type_info_set*& val = make_global_type_info_object();
			return *val;
		}

		template<typename t>
		void declare(std::string&& new_name, type_info_set& names = global_type_info())
		{
			auto ends_with = [](std::string const & value, std::string const & ending) -> bool
			{
				if (ending.size() > value.size()) return false;
				return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
			};

			assert_with_invalid_method_usage(!ends_with(new_name, "-ref") && !ends_with(new_name, "-const"));
			static_assert(!(std::is_const_v<t>||std::is_reference_v<t>), "can only declare a raw type, references and const are not allowed");

			names.operations[new_name] = &global_type_operation_adresses<t>;
			names.names[std::type_index{ typeid(t) }] = std::move(new_name);
			
		}
		
		template<typename t>
		std::string name_of(type_info_set const& names = global_type_info())
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
				auto g = names.names.find(std::type_index{ typeid(raw) });
				if (g == names.names.end())
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
