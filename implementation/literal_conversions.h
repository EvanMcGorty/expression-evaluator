#pragma once
#include<optional>
#include<string>
#include<sstream>
#include<iomanip>
#include<utility>
#include<limits>
#include"parsing_checks.h"
#include"type_qualities.h"

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

			if (start == stop)
			{
				return std::nullopt;
			}

			ret.first = 0;

			int curv;

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
					if ((std::numeric_limits<big_uint>::max() - curv) / 10 <= ret.first)
					{
						return std::nullopt;
					}
					ret.first *= 10;
					ret.first += curv;
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
						if ((std::numeric_limits<big_uint>::max() - curv) / 10 <= ret.second)
						{
							return std::nullopt;
						}
						ret.second *= 10;
						ret.second += curv;
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
						if ((std::numeric_limits<big_uint>::max() - curv) / 10 <= ret.first || std::numeric_limits<big_uint>::max() / 10 <= ret.second)
						{
							return std::nullopt;
						}
						ret.first *= 10;
						ret.first += curv;
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

		
		template<typename t>
		struct converter
		{
			//when successful, start is exactly one place ahead of the last character of the parsed value.
			static std::optional<t> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{
				if constexpr(type_wrap_info<t>::is())
				{
					return std::nullopt;
				}
				else if constexpr(std::is_arithmetic_v<t> && !std::is_same_v<t,bool>)
				{
					auto p = parse_range_to_number(start,stop);
					if (!p)
					{
						return std::nullopt;
					}
					bool is_negative = p->first;
					big_uint numerator = std::move(p->second.first);
					big_uint denomenator = std::move(p->second.second);
					t ret;


					if constexpr(!std::is_signed_v<t>)
					{
						if (is_negative)
						{
							return std::nullopt;
						}
					}


					if constexpr(std::is_integral_v<t>)
					{
						big_uint tm = (numerator / denomenator);
						if (tm > std::numeric_limits<t>::max())
						{
							return std::nullopt;
						}
						ret = static_cast<t>(tm);
					}
					else
					{
						if (numerator > std::numeric_limits<t>::max())
						{
							return std::nullopt;
						}
						ret = static_cast<t>(static_cast<t>(numerator) / denomenator);
					}

					if (is_negative)
					{
						ret *= -1;
					}

					return std::optional<t>{std::move(ret)};
				}
				else if constexpr(std::is_same_v<std::string, t>)
				{
					return std::optional<std::string>(std::string{ start,stop });
				}
				else
				{
					return std::nullopt;
				}
			}

			static std::string print(t const& tar)
			{
				if constexpr(type_wrap_info<t>::is())
				{
					if (type_wrap_info<t>::get(tar) == nullptr)
					{
						return "NULL";
					}
					else
					{
						return converter<typename type_wrap_info<t>::deref>::print(*tar);
					}
				}
				else if constexpr(std::is_arithmetic_v<t>)
				{
					std::stringstream s;
					s << tar;
					return s.str();
				}
				else if constexpr(std::is_convertible_v<t const&, std::string>)
				{
					return { tar };
				}
				else
				{
					std::stringstream s;
					s << std::hex << std::setw(2) << std::setfill('0') << "0x";
					unsigned char const* p = reinterpret_cast<unsigned char const*>(&tar);
					typedef unsigned int printable;
					for (int i = 0; i != sizeof(t); ++i)
					{
						s << std::setw(2) << printable{ p[i] };
					}
					return s.str();
				}
			}
		};

		template<typename t>
		struct converter<std::vector<t>>
		{
			static std::optional<std::vector<t>> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{
				if (start == stop)
				{
					return { std::vector<t>{} };
				}
				while (start != stop && *start == ' ')
				{
					++start;
				}
				if (start == stop)
				{
					return { std::vector<t>{} };
				}
				if (*start == '[')
				{
					std::vector<t> ret;
					++start;
					while (start != stop)
					{
						while (start != stop && *start == ' ')
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

						std::optional<t> curelem = converter<t>::parse(start, stop);
						if (curelem)
						{
							ret.emplace_back(std::move(*curelem));
						}
						else
						{
							return std::nullopt;
						}

						while (start != stop && *start == ' ')
						{
							++start;
						}
						if (start == stop)
						{
							return std::nullopt;
						}
						if (*start == ',')
						{
							++start;
						}
						else if (*start == ']')
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
				for (auto const& it : tar)
				{
					ret.append(converter<t>::print(it));
					ret.push_back(',');
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