#include"../../../implementation/function_sets.h"

namespace expr
{
    namespace impl
    {
        

		template<typename t>
		struct extended_util<std::vector<t>>
		{
			static object_holder from_list(std::vector<stack_elem>& a)
			{
				std::vector<t> ret;
				for (auto& v : a)
				{
					std::optional<val_wrap<t>> g = smart_take_elem<val_wrap<t>>(v);
					if (g)
					{
						ret.emplace_back(t(std::move(*g)));
					}
				}
				return make_object<std::vector<t>>(std::move(ret));
			}

			static t& index(std::vector<t>& tar, size_t at)
			{
				return tar.at(at);
			}

			static t const& const_index(std::vector<t> const& tar, size_t at)
			{
				return tar.at(at);
			}

			static void append(std::vector<t>& a, std::vector<t>&& b)
			{
				a.reserve(a.size() + b.size());
				for (auto&& i : std::move(b))
				{
					a.emplace_back(std::move(i));
				}
			}

			static void resize(std::vector<t>& a, size_t to)
			{
				if constexpr(std::is_default_constructible_v<t>)
				{
					a.resize(to);
				}
				else
				{
					assert_with_invalid_method_usage([&]() {return false; });
				}
			}

			static size_t size(std::vector<t> const& a)
			{
				return a.size();
			}

		};

		template<typename t>
		struct fs_info<extended_util<std::vector<t>>>
		{
			static function_set get_functions()
			{
				function_set ret;
				ret
					.add(mfn(&extended_util<std::vector<t>>::from_list), "make")
					.add(mfn(&extended_util<std::vector<t>>::from_list), "list-make")
					.add(sfn(&extended_util<std::vector<t>>::index), "at")
					.add(sfn(&extended_util<std::vector<t>>::const_index), "at")
					.add(sfn(&extended_util<std::vector<t>>::append), "append")
					.add(sfn(&extended_util<std::vector<t>>::size), "size");
				if constexpr(std::is_default_constructible_v<t>)
				{
					ret.add(sfn(&extended_util<std::vector<t>>::resize), "resize");
				}
				return std::move(ret);
			}

			static std::string get_name(type_info_set const* from)
			{
				return name_of<val_wrap<std::vector<t>>>(from);
			}
		};



		template<typename t>
		struct type_operation_info<std::vector<t>>
		{
			template<typename name_generator = compiler_name_generator>
			static std::string type_name(name_generator instance)
			{
				return instance.template retrieve<t>() + "-vector";
			}

			static std::optional<std::vector<t>> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{

				if (start == stop)
				{
					return { std::vector<t>{} };
				}
				while (start != stop && name_checker::iswhitespace(*start))
				{
					++start;
				}
				if (start == stop)
				{
					return { std::vector<t>{} };
				}
				if (*start == '[' || *start == '{')
				{
					char close_char = '!'; //not initialized
					if (*start == '[')
					{
						close_char = ']';
					}
					else if (*start == '{')
					{
						close_char = '}';
					}
					std::vector<t> ret;
					++start;
					while (start != stop)
					{
						while (start != stop && name_checker::iswhitespace(*start))
						{
							++start;
						}

						if (start == stop)
						{
							return std::nullopt;
						}
						else if (*start == ']')
						{
							++start;
							return { std::move(ret) };
						}

						std::optional<t> curelem = type_operation_info<t>::parse(start, stop);
						if (curelem)
						{
							ret.emplace_back(std::move(*curelem));
						}
						else
						{
							return std::nullopt;
						}

						while (start != stop && name_checker::iswhitespace(*start))
						{
							++start;
						}
						if (start == stop)
						{
							return std::nullopt;
						}
						if (*start == close_char)
						{
							++start;
							return { std::move(ret) };
						}
						continue; //the list does not require commas.
					}
					return std::nullopt;
				}
				else
				{
					return std::nullopt;
				}

			}

			static std::string print(std::vector<t> const& tar)
			{
				std::string ret;
				ret.push_back('[');
				for (auto& it : tar)
				{
					ret.append(type_operation_info<t>::print(it));
					ret.push_back(' ');
				}
				if (ret.size() == 1)
				{
					ret.push_back(' ');
				}
				(*ret.rbegin()) = ']';
				return std::move(ret);
			}
		};


    }
}