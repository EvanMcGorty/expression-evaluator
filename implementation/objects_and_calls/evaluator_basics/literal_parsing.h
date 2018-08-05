#pragma once
#include<optional>
#include<string>
#include<sstream>
#include<iomanip>
#include<utility>
#include<limits>

namespace expr
{
	namespace impl
	{
		using big_uint = unsigned long long;

		int getnum(char a)
		{
			switch (a)
			{
			case ('0'):
				return 0;
				break;
			case ('1'):
				return 1;
				break;
			case ('2'):
				return 2;
				break;
			case ('3'):
				return 3;
				break;
			case ('4'):
				return 4;
				break;
			case ('5'):
				return 5;
				break;
			case ('6'):
				return 6;
				break;
			case ('7'):
				return 7;
				break;
			case ('8'):
				return 8;
				break;
			case ('9'):
				return 9;
				break;
			case (' '):
				return -1;
				break;
			case ('/'):
				return -2;
				break;
			case ('.'):
				return -3;
				break;
			default:
				return -4;
				break;
			}
		}

		std::optional<std::pair<bool, std::pair<big_uint, big_uint>>> parse_range_to_number(std::string::const_iterator& start, std::string::const_iterator stop)
		{
			std::pair<big_uint, big_uint> ret;
			bool signret = false;
			while (start != stop && *start == ' ')
			{
				++start;
			}
			if (start == stop)
			{
				return std::nullopt;
			}

			if (*start == '-')
			{
				signret = true;
				++start;
			}

			if (start == stop || getnum(*start) == -4)
			{
				return std::nullopt;
			}


			ret.first = 0;

			int curv = -5; //to silence the warning

			while (start != stop)
			{
				curv = getnum(*start);
				if (curv < 0)
				{
					ret.second = 1;
					break;
				}
				else
				{
					if ((std::numeric_limits<big_uint>::max() - static_cast<big_uint>(curv)) / 10 <= ret.first)
					{
						return std::nullopt;
					}
					ret.first *= 10;
					ret.first += static_cast<big_uint>(curv);
				}
				++start;
			}

			if (start == stop || curv == -1)
			{
				ret.second = 1;
				return std::pair<bool, std::pair<big_uint, big_uint>>{signret, ret};
			}
			else if (curv == -2)
			{
				++start;
				if (start == stop || *start == ' ')
				{
					return std::nullopt;
				}
				ret.second = 0;
				while (start != stop)
				{
					curv = getnum(*start);
					if (curv < -1)
					{
						return std::make_pair(signret, ret);
					}
					else if (curv == -1)
					{
						break;
					}
					else
					{
						if ((std::numeric_limits<big_uint>::max() - static_cast<big_uint>(curv)) / 10 <= ret.second)
						{
							return std::nullopt;
						}
						ret.second *= 10;
						ret.second += static_cast<big_uint>(curv);
					}
					++start;
				}
			}
			else if (curv == -3)
			{
				++start;
				if (start == stop || *start == ' ')
				{
					return std::nullopt;
				}
				ret.second = 1;
				while (start != stop)
				{
					curv = getnum(*start);
					if (curv < -1)
					{
						return std::make_pair(signret, ret);
					}
					else if (curv == -1)
					{
						break;
					}
					else
					{
						if ((std::numeric_limits<big_uint>::max() - static_cast<big_uint>(curv)) / 10 <= ret.first || std::numeric_limits<big_uint>::max() / 10 <= ret.second)
						{
							return std::nullopt;
						}
						ret.first *= 10;
						ret.first += static_cast<big_uint>(curv);
						ret.second *= 10;
					}
					++start;
				}
			}
			else if (curv == -4)
			{
				return std::make_pair(signret,ret);
			}

			return std::pair<bool, std::pair<big_uint, big_uint>>{signret, ret};
		}

		std::optional<std::pair<bool, std::pair<big_uint, big_uint>>> parse_string_to_number(std::string const& a)
		{
			auto b = a.cbegin();
			auto p = parse_range_to_number(b, a.cend());
			if (!p)
			{
				return std::nullopt;
			}
			while (b != a.cend())
			{
				if (*b == ' ')
				{
					return std::nullopt;
				}
				++b;
			}
			return *p;
		}

		



	}
}
