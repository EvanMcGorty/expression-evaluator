#pragma once
#include<memory>
#include<iomanip>

#include"literal_parsing.h"
#include"type_demangle.h"

namespace expr
{
	namespace impl
	{
		struct wrapper_type_operation_trait
		{};
		
		template <typename t>
		struct type_operation_info
		{
			static std::string print(t const& tar)
			{
				if constexpr(std::is_arithmetic_v<t>)
				{
					std::stringstream s;

					if constexpr(std::is_floating_point_v<t>)
					{
						s << std::fixed << std::setprecision(32);
					}
					if constexpr(std::is_same_v<t, bool>)
					{
						s << std::boolalpha;
					}

					s << tar;
					std::string ret = s.str();

					if constexpr(std::is_floating_point_v<t>)
					{
						while (*ret.rbegin() != '.' && (*ret.rbegin() == '0' || ret.size() >= 15))
						{
							ret.pop_back();
						}
						if (*ret.rbegin() == '.')
						{
							ret.pop_back();
						}
					}
					return ret;
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

			template<typename name_generator = compiler_name_generator>
			static std::string type_name(name_generator instance)
			{
				return compiler_name_generator{}.retrieve<t>();
			}

			//when successful, start is exactly one place ahead of the last character of the parsed value.
			static std::optional<t> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{
				if constexpr(std::is_arithmetic_v<t> && !std::is_same_v<t, bool>)
				{
					auto p = parse_range_to_number(start, stop);
					if (!p)
					{
						return std::nullopt;
					}
					bool is_negative = p->first;
					big_uint numerator = std::move(p->second.first);
					big_uint denomenator = std::move(p->second.second);
					std::remove_const_t<t> ret;


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

					if constexpr(std::is_signed_v<t>)
					{
						if (is_negative)
						{
							ret *= -1;
						}
					}

					return std::optional<t>{std::move(ret)};
				}
				else if constexpr(std::is_same_v<t, bool>)
				{
					if (start == stop)
					{
						return std::nullopt;
					}
					if (*start == '1')
					{
						++start;
						return std::make_optional(true);
					}
					else if (*start == '0')
					{
						++start;
						return std::make_optional(true);
					}
					else
					{
						return std::nullopt;
					}
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

		};

		template <typename t>
		struct type_operation_info<t&&> : public type_operation_info<t&>
		{

		};

		template <typename t>
		struct type_operation_info<t&> : public type_operation_info<t const&>
		{

		};

		template <typename t>
		struct type_operation_info<t const&> : public type_operation_info<t>
		{
			
		};


		template <typename t>
		struct type_operation_info<t*> : public wrapper_type_operation_trait
		{

			template<typename name_generator = compiler_name_generator>
			static std::string type_name(name_generator instance)
			{
				return instance.template retrieve<t>() + "-ptr";
			}

			static std::optional<t*> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{
				return std::nullopt;
			}

			static std::string print(t* const& tar)
			{
				if (tar)
				{
					return "&" + type_operation_info<t>::print(*tar);
				}
				else
				{
					return "nullptr";
				}
			}

		};


		template<typename t>
		class strong
		{
		private:
			std::remove_const_t<t> mutable val;

			strong() = delete;
			strong(strong<t> const&) = delete;
			void operator=(strong<t> const&) = delete;

		public:



			strong(t&& a) :
				val(std::move(a))
			{
				static_assert(std::is_trivially_destructible_v<t>, "strong should only be used for types that are trivially destructible");
			}

			strong(strong<t>&& a) :
				val(std::move(a.val))
			{}

			~strong()
			{
				//destructor needed to make this non trivially destructible
			}

			std::remove_const_t<t>& operator*() const
			{
				return val;
			}

			operator bool() const
			{
				return true;
			}
		};

		template <typename t>
		struct type_operation_info<strong<t>> : public wrapper_type_operation_trait
		{

			template<typename name_generator = compiler_name_generator>
			static std::string type_name(name_generator instance)
			{
				return instance.template retrieve<t>() + "-strong";
			}

			static std::optional<strong<t>> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{
				auto&& g = type_operation_info<std::remove_const_t<t>>::parse(start, stop);
				if (g)
				{
					return strong<t>(std::move(*g));
				}
				else
				{
					return std::nullopt;
				}
			}

			static std::string print(strong<t> const& tar)
			{
				return type_operation_info<t>::print(*tar);
			}

		};



		template<typename t, typename enable = void>
		struct wrapper_type_operation_info
		{
			static constexpr bool is_wrapper()
			{
				return false;
			}
		};




	}
}
