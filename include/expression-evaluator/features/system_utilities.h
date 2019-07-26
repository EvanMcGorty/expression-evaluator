#pragma once
#include"../../../implementation/function_sets.h"

namespace expr
{
    namespace impl
    {
		struct terminal
		{

			[[ noreturn ]] static void exit()
			{
				throw 0;
			}

			static void sys(std::string a)
			{
				system(a.c_str());
			}

			static void print(std::string a)
			{
				std::cout << a << std::flush;
			}

			static void println(std::string a)
			{
				std::cout << a << '\n' << std::flush;
			}

			static std::string read()
			{
				std::string ret;
				std::getline(std::cin, ret);
				return ret;
			}

			static std::string input(std::string msg)
			{
				std::cout << msg << std::flush;
				std::string ret;
				std::getline(std::cin, ret);
				return ret;
			}

		};

		template<>
		struct fs_info<terminal>
		{
			static function_set get_functions()
			{
				function_set ret;
				ret
					.add(sfn(&terminal::exit), "quit")
					.add(sfn(&terminal::sys), "system")
					.add(sfn(&terminal::print), "print")
					.add(sfn(&terminal::println), "println")
					.add(sfn(&terminal::read), "read")
					.add(sfn(&terminal::input), "input");
				return ret;
			}

			static std::string get_name(type_info_set const*)
			{
				return "sys";
			}
		};
    }

	
	//utilities for interacting with the terminal and system
	using impl::terminal;
}