#pragma once
#include<vector>
#include<optional>
#include<iterator>

#include"expressions_basics/virtual-function-utilities/algebraic_virtual.h"
#include"expressions_basics/virtual-function-utilities/free-store_virtual.h"

#include"parsing_checks.h"

namespace expr
{
	namespace impl
	{

		class expression;



		class node
		{
		public:

			node() = default;
			node(node const&) = default;
			node(node&&) = default;

			virtual bool is_literal() const
			{
				return false;
			}

			virtual bool is_variable() const
			{
				return false;
			}

			virtual bool is_function() const
			{
				return false;
			}

			virtual expression duplicate() const = 0;

			virtual bool is_equal(node const* a) const = 0;

			virtual std::string make_string() const = 0;

			//all nodes also should have a static function called parse that returns an optional of itself.
			//parse takes a reference to a const iterator which is the location to begin the parsing. (first character to parse)
			//it is secondly passed a stop point const iterator that indicates where to not go past.
			//if the passed text is invalid, it will return std::nullopt. if


			virtual ~node()
			{

			}
		};

		struct literal_value
		{
			literal_value(std::string&& a)
			{
				val = a;
			}

			std::string val;
		};

		class literal : public node
		{
		public:


			literal(literal_value&& a) :
				data(std::move(a))
			{}


			bool is_literal() const final override
			{
				return true;
			}


			expression duplicate() const override;


			bool is_equal(node const* a) const override
			{
				return a->is_literal() && static_cast<literal const*>(a)->data.val == data.val;
			}


			std::string make_string() const override
			{
				std::string ret("\"");
				ret.reserve(size_t(double(data.val.length()) * 1.1) + 10);
				bool in_text = true;
				for (auto it = data.val.cbegin(); it != data.val.cend(); ++it)
				{
					switch (*it)
					{
					case('"'):
						if (in_text)
						{
							ret.append("\"q");
							in_text = false;
						}
						else
						{
							ret.push_back('q');
						}
						break;
					case('\t'):
						if (in_text)
						{
							ret.append("\"t");
							in_text = false;
						}
						else
						{
							ret.push_back('t');
						}
						break;
					case('\n'):
						if (in_text)
						{
							ret.append("\"n");
							in_text = false;
						}
						else
						{
							ret.push_back('n');
						}
						break;
					case('\b'):
						if (in_text)
						{
							ret.append("\"b");
							in_text = false;
						}
						else
						{
							ret.push_back('b');
						}
						break;
					case('\a'):
						if (in_text)
						{
							ret.append("\"a");
							in_text = false;
						}
						else
						{
							ret.push_back('a');
						}
						break;
					case('\v'):
						if (in_text)
						{
							ret.append("\"v");
							in_text = false;
						}
						else
						{
							ret.push_back('v');
						}
						break;
					case('\r'):
						if (in_text)
						{
							ret.append("\"r");
							in_text = false;
						}
						else
						{
							ret.push_back('r');
						}
						break;
					case('\0'):
						if (in_text)
						{
							ret.append("\"0");
							in_text = false;
						}
						else
						{
							ret.push_back('0');
						}
						break;
					case('\f'):
						if (in_text)
						{
							ret.append("\"f");
							in_text = false;
						}
						else
						{
							ret.push_back('f');
						}
						break;
					default:
						if (in_text)
						{
							ret.push_back(*it);
						}
						else
						{
							ret.push_back('"');
							ret.push_back(*it);
							in_text = true;
						}
					}
				}
				if (!in_text)
				{
					ret.push_back('"');
				}
				ret.push_back('"');
				return ret;
			}

			template<typename IteratorToChar>
			static std::optional<literal> parse(IteratorToChar& start, IteratorToChar stop)
			{

				std::string ret;

				assert_with_generic_logic_error([&]() {return start != stop; });

				if(*start != '"')
				{
					if (*start == '\'')
					{
						++start;
					}
					while (start != stop && !name_checker::mustbeendofelem(*start))
					{
						ret.push_back(*start);
						++start;
					}
					while (ret.size() > 0 && name_checker::iswhitespace(*ret.rbegin()))
					{
						ret.pop_back();
					}
					return std::optional<literal>{literal{ std::move(ret) }};
				}



				++start;

				while (start != stop)
				{
					if (*start == '"')
					{
						++start;
						if (start == stop || name_checker::canbeendofelem(*start))
						{
							return std::optional<literal>{literal{std::move(ret)}};
						}
						while (start != stop)
						{
							if (*start == '"')
							{
								break;
							}
							else
							{
								switch (*start)
								{
								case('n'):
									ret.push_back('\n');
									break;
								case('t'):
									ret.push_back('\t');
									break;
								case('v'):
									ret.push_back('\v');
									break;
								case('b'):
									ret.push_back('\b');
									break;
								case('f'):
									ret.push_back('\f');
									break;
								case('a'):
									ret.push_back('\a');
									break;
								case('0'):
									ret.push_back('\0');
									break;
								case('q'):
									ret.push_back('"');
									break;
								default:
									return std::nullopt;
								}
							}
							++start;
						}
						if (start == stop)
						{
							return std::nullopt;
						}
					}
					else
					{
						ret.push_back(*start);
					}
					++start;
				}
				return std::nullopt;
			}




			literal_value data;
		};

		enum class sc
		{
			pop,
			neutral,
			push
		};

		struct variable_value
		{
			variable_value(name_checker&& a, sc b)
			{
				change = b;
				var_name = std::string(std::move(a));
			}

			sc change;

			std::string var_name;
		};

		class variable : public node
		{
		public:

			variable(variable_value&& a) :
				data(std::move(a))
			{}


			bool is_variable() const final override
			{
				return true;
			}

			expression duplicate() const override;


			bool is_equal(node const* a) const override
			{
				return a->is_variable() && static_cast<variable const*>(a)->data.change == data.change && static_cast<variable const*>(a)->data.var_name == data.var_name;
			}


			std::string make_string() const override
			{
				std::string ret;

				switch (data.change)
				{
				case(sc::neutral):
					ret = "=" + data.var_name;
					break;
				case(sc::pop):
					ret.reserve(data.var_name.size() + 2);
					ret.push_back('=');
					ret.append(data.var_name);
					ret.push_back('\\');
					break;
				case(sc::push):
					ret.reserve(data.var_name.size() + 2);
					ret.push_back('=');
					ret.append(data.var_name);
					ret.push_back('/');
					break;
				}
				return ret;
			}

			template<typename IteratorToChar>
			static std::optional<variable> parse(IteratorToChar& start, IteratorToChar stop)
			{
				assert_with_generic_logic_error([&]() {return start != stop && *start == '='; });

				std::string ret;
				sc sc_ret;

				++start;
				if (start == stop || !(name_checker::isupper(*start) || name_checker::islower(*start)))
				{
					return std::nullopt;
				}

				while (start != stop)
				{
					if (!(name_checker::isinnerchar(*start)))
					{
						char cur = *start;
						if (cur == '/')
						{
							sc_ret = sc::push;
							++start;
						}
						else if (cur == '\\')
						{
							sc_ret = sc::pop;
							++start;
						}
						else if (name_checker::mustbeendofelem(cur))
						{
							sc_ret = sc::neutral;
						}
						else
						{
							return std::nullopt;
						}

						if (ret.size() == 0 || !name_checker::isendchar(ret[ret.size() - 1]))
						{
							return std::nullopt;
						}
						else
						{
							return std::optional<variable>{variable{ variable_value{std::move(ret),sc_ret} }};
						}
					}
					ret.push_back(*start);
					++start;
				}

				if (ret.size() == 0 || name_checker::isotherinner(ret[ret.size() - 1]))
				{
					return std::nullopt;
				}
				else
				{
					return std::optional<variable>{variable{ variable_value{std::move(ret),sc::neutral} }};
				}
			}



			variable_value data;
		};


		struct function_value
		{
			function_value(name_checker&& n, std::vector<expression>&& a = {})
			{
				fn_name = std::move(n);
				arguments = std::move(a);
			}

			std::string fn_name;

			std::vector<expression> arguments;
		};

		class function : public node
		{
		public:

			function(function_value&& a) :
				data(std::move(a))
			{}



			bool is_function() const final override
			{
				return true;
			}

			expression duplicate() const override;


			bool is_equal(node const* a) const override;


			std::string make_string() const override;

			template<typename IteratorToChar>
			static std::optional<function> parse(IteratorToChar& start, IteratorToChar stop);



			function_value data;

		};


		//using dtp = mu::algebraic<node,literal,variable,function>;

		using dtp = mu::algebraic<node, literal, variable, function>;

		class executable;

		
		//reading into raw_char calls istream::get(char&) so it can be read newlines
		struct raw_char
		{
			operator char&&() &&
			{
				return std::move(val);
			}
			operator char&()
			{
				return val;
			}
			operator char const&() const
			{
				return val;
			}

			char val;
		};

		std::istream& operator>>(std::istream& from, raw_char& to)
		{
			from.get(to.val);
			return from;
		}

		std::istream_iterator<raw_char> raw_istream_iter()
		{
			return std::istream_iterator<raw_char>();
		}

		std::istream_iterator<raw_char> raw_istream_iter(std::istream& target)
		{
			return std::istream_iterator<raw_char>(target);
		}

		class expression
		{
			friend expression literal::duplicate() const;
			friend expression variable::duplicate() const;
			friend expression function::duplicate() const;
			friend std::string function::make_string() const;

		public:

			expression() noexcept(true) :
				val(dtp::make_nullval())
			{}

			expression(expression&& a) noexcept(true) :
				val(std::move(a.val))
			{
			}

			void operator=(expression&& a) noexcept(true)
			{
				val = std::move(a.val);
			}

			expression(expression const& a) :
				val(!a.val.is_nullval() ? std::move(a.val->duplicate().val) : dtp::make_nullval())
			{}

			void operator=(expression const& a)
			{
				val = (!a.val.is_nullval() ? std::move(a.val->duplicate().val) : dtp::make_nullval());
			}

			bool operator==(expression const& a) const
			{
				if (val.is_nullval())
				{
					if (a.val.is_nullval())
					{
						return true;
					}
					else
					{
						return false;
					}
				}
				else
				{
					if (a.val.is_nullval())
					{
						return false;
					}
					else
					{
						return val->is_equal(&*a.val);
					}
				}
			}

			bool operator!=(expression const& a) const
			{
				return !operator==(a);
			}


			std::string str() const
			{
				if (val.is_nullval())
				{
					return "_";
				}
				else
				{
					return val->make_string();
				}
			}


			void make_executable(executable& a) const;

			void into_executable(executable& a) &&;

			static expression make(literal_value&& a)
			{
				return expression{ dtp::make<literal>(std::move(a)) };
			}

			static expression make(variable_value&& a)
			{
				return expression{ dtp::make<variable>(std::move(a)) };
			}

			static expression make(function_value&& a)
			{
				return expression{ dtp::make<function>(std::move(a)) };
			}

			static expression make_empty()
			{
				return expression{ dtp::make_nullval() };
			}

			static expression make(std::string const& to_be_parsed)
			{
				std::string::const_iterator it = to_be_parsed.cbegin();
				std::optional<expression> o{ parse(it,to_be_parsed.cend()) };
				if (!o)
				{
					return make_empty();
				}
				while (it != to_be_parsed.cend())
				{
					if (!name_checker::iswhitespace(*it))
					{
						return make_empty();
					}
					++it;
				}
				return std::move(*o);
			}

			static expression make(char const* to_be_parsed)
			{
				return make(std::string{ to_be_parsed });
			}

			template<typename IteratorToChar>
			static std::optional<expression> parse(IteratorToChar& start, IteratorToChar stop)
			{
				if (start == stop)
				{
					return std::nullopt;
				}
				if (*start == '=')
				{
					std::optional<variable> n = variable::parse(start, stop);
					if (n)
					{
						return std::optional<expression>{expression{ dtp::make<variable>(std::move(*n)) }};
					}
					else
					{
						return std::nullopt;
					}
				}
				else if (name_checker::isupper(*start) || name_checker::islower(*start))
				{
					std::optional<function> n{ function::parse(start,stop) };
					if (n)
					{
						return std::optional<expression>{expression{ dtp::make<function>(std::move(*n)) }};
					}
					else
					{
						return std::nullopt;
					}
				}
				else if (*start == '_')
				{
					++start;
					return std::optional<expression>{expression::make_empty()};
				}
				else
				{
					std::optional<literal> n = literal::parse(start, stop);
					if (n)
					{
						return std::optional<expression>{expression{ dtp::make<literal>(std::move(*n)) }};
					}
					else
					{
						return std::nullopt;
					}
				}
			}

			template<typename IteratorToChar>
			static expression literal_parse(IteratorToChar& start, IteratorToChar stop)
			{
				auto g = literal::parse(start, stop);
				if (g)
				{
					return expression::make(std::move(g->data));
				}
				else
				{
					return expression::make_empty();
				}
			}

			template<typename IteratorToChar>
			static expression variable_parse(IteratorToChar& start, IteratorToChar stop)
			{
				auto g = variable::parse(start, stop);
				if (g)
				{
					return expression::make(std::move(g->data));
				}
				else
				{
					return expression::make_empty();
				}
			}

			template<typename IteratorToChar>
			static expression function_parse(IteratorToChar& start, IteratorToChar stop)
			{
				auto g = function::parse(start, stop);
				if (g)
				{
					return expression::make(std::move(g->data));
				}
				else
				{
					return expression::make_empty();
				}
			}


			literal_value* get_literal()
			{
				if (!val.is_nullval() && val->is_literal())
				{
					return &val.downcast_get<literal>()->data;
				}
				else
				{
					return nullptr;
				}
			}

			variable_value* get_variable()
			{
				if (!val.is_nullval() && val->is_variable())
				{
					return &val.downcast_get<variable>()->data;
				}
				else
				{
					return nullptr;
				}
			}

			function_value* get_function()
			{
				if (!val.is_nullval() && val->is_function())
				{
					return &val.downcast_get<function>()->data;
				}
				else
				{
					return nullptr;
				}
			}


			literal_value const* get_literal() const
			{
				if (!val.is_nullval() && val->is_literal())
				{
					return &val.downcast_get<literal>()->data;
				}
				else
				{
					return nullptr;
				}
			}

			variable_value const* get_variable() const
			{
				if (!val.is_nullval() && val->is_variable())
				{
					return &val.downcast_get<variable>()->data;
				}
				else
				{
					return nullptr;
				}
			}

			function_value const* get_function() const
			{
				if (!val.is_nullval() && val->is_function())
				{
					return &val.downcast_get<function>()->data;
				}
				else
				{
					return nullptr;
				}
			}



		private:

			dtp val;


			expression(dtp&& a) :
				val(std::move(a))
			{}
		};

		std::ostream& operator<<(std::ostream& stream, expression const& expression)
		{
			stream << expression.str();
			return stream;
		}

		std::istream& operator>>(std::istream& stream, expression& expression)
		{
			auto it = raw_istream_iter(stream);
			expression = expression::parse(it, raw_istream_iter()).value_or(expression::make_empty());
			return stream;
		}

		inline std::string function::make_string() const
		{
			std::string ret;
			ret.reserve(data.fn_name.size() + data.arguments.size() * 10);
			ret = data.fn_name;
			ret.push_back('(');
			for (auto const& it : data.arguments)
			{
				if (it.val.is_nullval())
				{
					ret.push_back('_');
				}
				else
				{
					ret.append(it.str());
				}
				ret.push_back(',');
			}
			if (data.arguments.size() > 0)
			{
				ret.pop_back();
			}

			ret.push_back(')');

			return ret;
		}

		template<typename IteratorToChar>
		inline std::optional<function> function::parse(IteratorToChar& start, IteratorToChar stop)
		{
			assert_with_generic_logic_error([&]() {return start != stop && (name_checker::isupper(*start) || name_checker::islower(*start)); });


			std::string retname;
			while (start != stop && name_checker::isinnerchar(*start))
			{
				retname.push_back(*start);
				++start;
			}
			if (!name_checker::isendchar(*(retname.end() - 1)))
			{
				return std::nullopt;
			}
			function ret{ name_checker{std::move(retname)} };

			if (start != stop && *start == '\n')
			{
				return std::optional<function>{std::move(ret)};
			}

			while (start != stop && name_checker::iswhitespace(*start))
			{
				++start;
			}

			if (start == stop || *start != '(')
			{
				return std::optional<function>{std::move(ret)};
			}

			bool on_first_iteration = true;

			while (start != stop && *start != ')')
			{
				++start;

				while (start != stop && name_checker::iswhitespace(*start))
				{
					++start;
				}

				if (start == stop)
				{
					return std::nullopt;
				}

				if (*start == ')')
				{
					if (!on_first_iteration)
					{
						return std::nullopt;
					}
					++start;
					return std::optional<function>{std::move(ret)};
				}

				auto next = expression::parse(start, stop);

				if (next)
				{
					ret.data.arguments.emplace_back(std::move(*next));
				}
				else
				{
					return std::nullopt;
				}

				while (start != stop && name_checker::iswhitespace(*start))
				{
					++start;
				}

				if (start != stop && !name_checker::mustbeendofelem(*start))
				{
					return std::nullopt;
				}

				on_first_iteration = false;
			}

			if (start == stop)
			{
				return std::nullopt;
			}

			++start;

			return std::optional<function>{std::move(ret)};

		}

		bool function::is_equal(node const *a) const
		{
			if (a->is_function() && static_cast<function const *>(a)->data.fn_name == data.fn_name)
			{
				std::vector<expression> const &r = static_cast<function const *>(a)->data.arguments;
				if (r.size() != data.arguments.size())
				{
					return false;
				}
				for (size_t i = 0; i != data.arguments.size(); ++i)
				{
					if (r[i] != data.arguments[i])
					{
						return false;
					}
				}
				return true;
			}
			else
			{
				return false;
			}
		}

		inline expression literal::duplicate() const
		{
			return expression{ dtp::make<literal>(std::move(*this)) };
		}

		inline expression variable::duplicate() const
		{
			return expression{ dtp::make<variable>(std::move(*this)) };
		}

		inline expression function::duplicate() const
		{
			return expression{ dtp::make<function>(std::move(*this)) };
		}

	}

}
