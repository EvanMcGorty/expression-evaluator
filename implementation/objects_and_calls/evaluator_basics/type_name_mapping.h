#include <string>
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <cstdlib>

#include "type_operation_info.h"
#include "call_typing.h"

namespace expr
{
	namespace impl
	{
		struct predeclared_object_result;

		class type_operations
		{
		public:
			virtual predeclared_object_result make_from_string(std::string::const_iterator& start, std::string::const_iterator stop) const = 0;
		};

		template<typename t>
		class type_operations_for : public type_operations
		{
		public:
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
		void declare_with_name(std::string&& new_name, type_info_set& names = global_type_info())
		{
			auto ends_with = [](std::string const & value, std::string const & ending) -> bool
			{
				if (ending.size() > value.size()) return false;
				return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
			};

			assert_with_invalid_method_usage([&]() {return !ends_with(new_name, "-ref") && !ends_with(new_name, "-const"); });
			static_assert(!(std::is_const_v<t>||std::is_reference_v<t>), "can only declare a raw type, references and const are not allowed");
			
			auto g = names.names.find(std::type_index{ typeid(t) });
			if (g != names.names.end())
			{
				names.operations.erase(names.operations.find(g->second));
				names.names.erase(g);
			}
			names.operations[new_name] = &global_type_operation_adresses<t>;
			names.names[std::type_index{ typeid(t) }] = std::move(new_name);
			
		}


		struct type_info_set_name_generator
		{

			type_info_set_name_generator(type_info_set const& a)
			{
				names = &a;
			}

			template<typename t>
			std::string retrieve();

			type_info_set const* names;
		};
		
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
					static_assert(!std::is_const_v<raw>);
					std::string ret = type_operation_info<raw>::template type_name<type_info_set_name_generator>(type_info_set_name_generator{ names });

					while (names.operations.find(ret) != names.operations.end())
					{
						ret = "automatically_named." + ret;
					}
					return ret;
				}
				else
				{
					return std::string{ g->second };
				}
			}
		}


		template<typename t, typename...ts>
		void declare(type_info_set& names = global_type_info())
		{
			declare_with_name<t>(name_of<pre_call_t<t>>(names));
			if constexpr(sizeof...(ts) > 0)
			{
				declare<ts...>(names);
			}
		}

		template<typename t>
		inline std::string type_info_set_name_generator::retrieve()
		{
			return name_of<pre_call_t<t>>(*names);
		}

	}
}
