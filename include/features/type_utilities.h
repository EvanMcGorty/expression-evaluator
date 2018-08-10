#include"../../implementation/function_sets.h"

namespace expr
{
    namespace impl
    {
		struct types
		{
			types(type_info_set const& a)
			{
				names = &a;
			}

			auto parse_string()
			{
				auto ret = [names = names](std::vector<stack_elem>& a) -> object_holder
				{
					if(a.size() == 1)
					{
						std::optional<ref_wrap<std::string const>> g = smart_take_elem<ref_wrap<std::string const>>(a[0]);
						if(g)
						{
							auto start = g->to->begin();
							auto stop = g->to->end();
							return parse_to_object(start,stop,*names);
						}
					}
					return object_holder::make_nullval();
				};
				return ret;
			}

			auto to_string()
			{
				auto ret = [names = names](std::vector<stack_elem>& a) -> object_holder
				{
					if(a.size() == 1 && !a[0].is_nullval())
					{
						return make_object(a[0]->string_view(*names));
					}
					return object_holder::make_nullval();
				};
				return ret;
			}

			type_info_set const* names;
		};

		template<>
		struct fs_info<types>
		{
			static function_set get_functions(type_info_set const& names = global_type_info())
			{
				types tar{names};
				function_set ret;
				ret
					.add(mfn(tar.to_string()), "obj_to_str")
					.add(mfn(tar.parse_string()), "str_to_obj");
				return ret;
			}

			static std::string get_name(type_info_set const&)
			{
				return "types";
			}
		};
	}

	using impl::types;
}