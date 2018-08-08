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
				else if constexpr(std::is_convertible_v<t const&, std::string>)
				{
					return literal{ literal_value{ std::string(tar) } }.make_string();
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

		template <typename t>
		struct type_operation_info<std::unique_ptr<t>> : public wrapper_type_operation_trait
		{

			template<typename name_generator = compiler_name_generator>
			static std::string type_name(name_generator instance)
			{
				return instance.template retrieve<t>() + "-unique_ptr";
			}

			static std::optional<std::unique_ptr<t>> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{
				if (start == stop)
				{
					//invalid input
					return std::optional<std::unique_ptr<t>>(nullptr);
				}

				static char const none[] = "nullptr";

				auto m = std::mismatch(start, stop, std::begin(none), std::end(none) - 1);

				if (m.second == std::end(none) - 1)
				{
					start = m.first;
					return std::optional<std::unique_ptr<t>>(std::unique_ptr<t>(nullptr));
				}
				else if (*start == '&')
				{
					++start;
					std::optional<t>&& g = type_operation_info<t>::parse(start, stop);
					if (g)
					{
						return std::optional<std::unique_ptr<t>>(std::make_unique<t>(std::move(*g)));
					}
					else
					{
						return std::nullopt;
					}
				}
				else
				{
					return std::nullopt;
				}
			}

			static std::string print(std::unique_ptr<t> const& tar)
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

		template <typename t>
		struct type_operation_info<std::shared_ptr<t>> : public wrapper_type_operation_trait
		{

			template<typename name_generator = compiler_name_generator>
			static std::string type_name(name_generator instance)
			{
				return instance.template retrieve<t>() + "-shared_ptr";
			}

			static std::optional<std::shared_ptr<t>> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{
				if (start == stop)
				{
					//invalid input
					return std::optional<std::shared_ptr<t>>(nullptr);
				}

				static char const none[] = "nullptr";

				auto m = std::mismatch(start, stop, std::begin(none), std::end(none) - 1);

				if (m.second == std::end(none) - 1)
				{
					start = m.first;
					return std::optional<std::shared_ptr<t>>(std::shared_ptr<t>(nullptr));
				}
				else if (*start == '&')
				{
					++start;
					std::optional<t>&& g = type_operation_info<t>::parse(start, stop);
					if (g)
					{
						return std::optional<std::shared_ptr<t>>(std::make_unique<t>(std::move(*g)));
					}
					else
					{
						return std::nullopt;
					}
				}
				else
				{
					return std::nullopt;
				}
			}

			static std::string print(std::shared_ptr<t> const& tar)
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


		template <typename t>
		struct type_operation_info<std::optional<t>> : public wrapper_type_operation_trait
		{

			static std::string print(std::optional<t> const& tar)
			{
				if (tar)
				{
					return "?" + type_operation_info<t const&>::print(*tar);
				}
				else
				{
					return "nullopt";
				}
			}



			template<typename name_generator = compiler_name_generator>
			static std::string type_name(name_generator instance)
			{
				return instance.template retrieve<t>() + "-optional";
			}



			static std::optional<std::optional<t>> parse(std::string::const_iterator& start, std::string::const_iterator stop)
			{
				if (start == stop)
				{
					//invalid input
					return std::optional<std::optional<t>>(std::nullopt);
				}

				static char const none[] = "nullopt";

				auto m = std::mismatch(start, stop, std::begin(none), std::end(none) - 1);

				if (m.second == std::end(none) - 1)
				{
					//valid input indicating an empty optional of t.
					start = m.first;
					return std::optional<std::optional<t>>(std::optional<t>(std::nullopt));
				}
				else if (*start == '?')
				{
					++start;
					std::optional<t>&& g = type_operation_info<t>::parse(start, stop);
					if (g)
					{
						return std::optional<std::optional<t>>(std::move(g));
					}
					else
					{
						return std::nullopt;
					}
				}
				else
				{
					return std::nullopt;
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


		template<typename t>
		struct type_operation_info<std::vector<t> const&>
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
				while (start != stop && *start == ' ')
				{
					++start;
				}
				if (start == stop)
				{
					return { std::vector<t>{} };
				}
				if (*start == '[' || *start == '(' || *start == '{')
				{
					char close_char = '!'; //not initialized
					if (*start == '[')
					{
						close_char = ']';
					}
					else if (*start == '(')
					{
						close_char = ')';
					}
					else if (*start == '{')
					{
						close_char = '}';
					}
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

						std::optional<t> curelem = type_operation_info<t>::parse(start, stop);
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

		template<>
		struct type_operation_info<std::string const&>
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





		template<typename t, typename enable = void>
		struct wrapper_type_operation_info
		{
			static constexpr bool is_wrapper()
			{
				return false;
			}
		};

		template<typename t>
		struct wrapper_type_operation_info<t,std::enable_if_t<std::is_base_of_v<wrapper_type_operation_trait, type_operation_info<t>>>>
		{
			static constexpr bool is_wrapper()
			{
				return true;
			}

			using wrapped = decltype(*std::declval<t>());

			static bool can_unwrap(t&& a)
			{
				if (std::forward<t>(a))
				{
					return true;
				}
				else
				{
					return false;
				}
			}

			static wrapped do_unwrap(t&& a)
			{
				return *std::forward<t>(a);
			}
		};



	}
}
