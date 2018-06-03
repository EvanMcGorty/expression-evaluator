#pragma once
#include<string>

namespace expr
{
	namespace impl
	{
		using expression_evaluator_internal_logic_error = std::logic_error;

		using expression_evaluator_usage_logic_error = std::logic_error;

#ifdef _DEBUG
		void assert_with_generic_logic_error(bool a)
		{
			if (!a)
			{
				throw expression_evaluator_internal_logic_error("the expression evaluator has an internal logic error");
			}
		}
		void throw_invalid_method_usage(bool a)
		{
			if (!a)
			{
				throw expression_evaluator_usage_logic_error("usage of invalid method");
			}
		}
		void throw_invalid_name_usage(bool a)
		{
			if (!a)
			{
				throw expression_evaluator_usage_logic_error("usage of invalid name");
			}
		}
#else
		void assert_with_generic_logic_error(bool){}
		void throw_invalid_method_usage(bool){}
		void throw_invalid_name_usage(bool){}
#endif



		//for passing a string while ensuring that it is a valid name with assert()
		//or for checking manually
		class name_checker
		{
		public:

			name_checker(char const* a)
			{
				name_checker(std::string{ a });
			}

			name_checker(std::string&& a)
			{
				assert_with_generic_logic_error(is_valid(a));
				held = std::move(a);
			}

			void operator=(char const* a)
			{
				name_checker(std::string{ a });
			}

			void operator=(std::string&& a)
			{
				assert_with_generic_logic_error(is_valid(a));
				held = std::move(a);
			}

			operator std::string() &&
			{
				return std::move(held);
			}

			name_checker(name_checker&& a)
			{
				held = std::move(a.held);
			}

			void operator=(name_checker&& a)
			{
				held = std::move(a.held);
			}

			std::string get_copy() const
			{
				return std::string(held);
			}

			//starts with a character that is uppercase or lower case
			//has middle characters that are uppercase, lowercase, digits, dashes, underscores, or dots.
			//ends with a character that is uppercase, lowercase, or a digit.
			static bool is_valid(std::string const& a)
			{

				if (!(a.length() > 0))
				{
					return false;
				}

				if (!isbeginchar(a[0]))
				{
					return false;
				}

				for (size_t i = 1; i != a.length(); ++i)
				{
					if (i + 1 == a.length())
					{
						if (!isendchar(a[i]))
						{
							return false;
						}
					}
					else
					{
						if (!isinnerchar(a[i]))
						{
							return false;
						}
					}
				}
				return true;
			}

			static bool isbeginchar(char a)
			{
				return islower(a) || isupper(a);
			}

			static bool isinnerchar(char a)
			{
				return islower(a) || isupper(a) || isdigit(a) || isotherinner(a);
			}

			static bool isendchar(char a)
			{
				return islower(a) || isupper(a) || isdigit(a);
			}

			static bool isupper(char a)
			{
				switch (a)
				{
				case('A'):case('B'):case('C'):case('D'):case('E'):case('F'):case('G'):case('H'):case('I'):case('J'):case('K'):case('L'):case('M'):case('N'):case('O'):case('P'):case('Q'):case('R'):case('S'):case('T'):case('U'):case('V'):case('W'):case('X'):case('Y'):case('Z'):
					return true;
					break;
				default:
					return false;
					break;
				}
			}

			static bool islower(char a)
			{
				switch (a)
				{
				case('a'):case('b'):case('c'):case('d'):case('e'):case('f'):case('g'):case('h'):case('i'):case('j'):case('k'):case('l'):case('m'):case('n'):case('o'):case('p'):case('q'):case('r'):case('s'):case('t'):case('u'):case('v'):case('w'):case('x'):case('y'):case('z'):
					return true;
					break;
				default:
					return false;
					break;
				}
			}

			static bool isdigit(char a)
			{
				switch (a)
				{
				case('0'):case('1'):case('2'):case('3'):case('4'):case('5'):case('6'):case('7'):case('8'):case('9'):
					return true;
					break;
				default:
					return false;
					break;
				}
			}

			static bool isotherinner(char a)
			{
				switch (a)
				{
				case('_'):case('-'):case('.'):
					return true;
					break;
				default:
					return false;
					break;
				}
			}

			static bool canbeafterelem(char a)
			{
				switch (a)
				{
				case(' '):case(','):case(')'):
					return true;
					break;
				default:
					return false;
					break;
				}
			}

		private:

			std::string held;

		};

	}
}
