#pragma once
#include"../../../implementation/function_sets.h"

namespace expr
{
    namespace impl
    {
        
		template<>
		struct extended_util<std::string>
		{
			static std::string from_char_vector(std::vector<char> const& a)
			{
				return std::string{ a.begin(),a.end() };
			}

			static std::string from_c_str(char const* from)
			{
				return std::string{ from };
			}

			static char const* as_c_str(std::string const& tar)
			{
				return tar.c_str();
			}

			static char& index(std::string& tar, size_t at)
			{
				return tar.at(at);
			}

			static char const& const_index(std::string const& tar, size_t at)
			{
				return tar.at(at);
			}

			static void append(std::string& a, std::string const& b)
			{
				a.append(b);
			}

			static void resize(std::string& a, size_t to)
			{
				a.resize(to);
			}

			static size_t size(std::string const& a)
			{
				return a.size();
			}

		};

		template<>
		struct fs_info<extended_util<std::string>>
		{
			static function_set get_functions()
			{
				function_set ret;
				ret
					.add(sfn(&extended_util<std::string>::from_char_vector), "char-vec-make")
					.add(sfn(&extended_util<std::string>::from_char_vector), "make")
					.add(sfn(&extended_util<std::string>::as_c_str), "as-c_str")
					.add(sfn(&extended_util<std::string>::from_c_str), "c_str-make")
					.add(sfn(&extended_util<std::string>::from_c_str), "make")
					.add(sfn(&extended_util<std::string>::index), "at")
					.add(sfn(&extended_util<std::string>::const_index), "at")
					.add(sfn(&extended_util<std::string>::append), "append")
					.add(sfn(&extended_util<std::string>::resize), "resize")
					.add(sfn(&extended_util<std::string>::size), "len");
				return ret;
			}

			static std::string get_name(type_info_set const* from)
			{
				return name_of<val_wrap<std::string>>(from);
			}
		};

		

		template<>
		struct type_operation_info<std::string>
		{
			template<typename name_generator = compiler_name_generator>
			static std::string type_name(name_generator instance)
			{
				return "string";
			}

			static std::optional<std::string> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{
				return std::optional<std::string>{ std::string(start,stop) };
			}

			static std::string print(std::string const& a)
			{
				return std::string(a);
			}
		};

		
    }
}