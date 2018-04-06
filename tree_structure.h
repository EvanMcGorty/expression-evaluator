#pragma once
#include<vector>
#include<optional>

#include"virtual-function-utilities\algebraic_virtual.h"
#include"virtual-function-utilities\free-store_virtual.h"

#include"parsing_checks.h"

namespace expressions
{

	class elem;



	class node
	{
	public:

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

		virtual elem duplicate() const = 0;

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
			value = a;
		}

		std::string value;
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


		elem duplicate() const override;


		bool is_equal(node const* a) const override
		{
			return a->is_literal() && static_cast<literal const*>(a)->data.value == data.value;
		}


		std::string make_string() const override
		{
			std::string ret("\"");
			ret.reserve(size_t(float(data.value.length()) * 1.1) + 10);
			bool in_text = true;
			for (auto it = data.value.cbegin(); it != data.value.cend(); ++it)
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


		static std::optional<literal> parse(std::string::const_iterator& start, std::string::const_iterator stop)
		{

			std::string ret;

			assert(start != stop);

			if (*start != '"')
			{
				while (start != stop && !name_checker::canbeafterelem(*start))
				{
					ret.push_back(*start);
					++start;
				}
				return std::optional<literal>{literal{std::move(ret)}};
			}



			++start;

			while (start != stop)
			{
				if (*start == '"')
				{
					++start;
					if (start == stop || name_checker::canbeafterelem(*start))
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

		elem duplicate() const override;


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
				ret = "@" + data.var_name;
				break;
			case(sc::pop):
				ret.reserve(data.var_name.size() + 2);
				ret.push_back('@');
				ret.append(data.var_name);
				ret.push_back('~');
				break;
			case(sc::push):
				ret.reserve(data.var_name.size() + 2);
				ret.push_back('@');
				ret.append(data.var_name);
				ret.push_back('+');
				break;
			}
			return ret;
		}


		static std::optional<variable> parse(std::string::const_iterator& start, std::string::const_iterator stop)
		{
			assert(start != stop && *start == '@');

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
					switch (*start)
					{
					case('+'):
						sc_ret = sc::push;
						++start;
						break;
					case('~'):
						sc_ret = sc::pop;
						++start;
						break;
					case(')'):
					case(','):
					case(' '):
						sc_ret = sc::neutral;
						break;
					default:
						return std::nullopt;
						break;
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
		function_value(name_checker&& n, std::vector<elem>&& a = {})
		{
			fn_name = std::move(n);
			arguments = std::move(a);
		}

		std::string fn_name;

		std::vector<elem> arguments;
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

		elem duplicate() const override;


		bool is_equal(node const* a) const override;


		std::string make_string() const override;

		static std::optional<function> parse(std::string::const_iterator& start, std::string::const_iterator stop);



		function_value data;

	};


	//using dtp = mu::algebraic<node,literal,variable,function>;

	using dtp = mu::algebraic<node, literal, variable, function>;

	class executable;

	class elem
	{
		friend elem literal::duplicate() const;
		friend elem variable::duplicate() const;
		friend elem function::duplicate() const;
		friend std::string function::make_string() const;

	public:

		elem() noexcept(true) :
			val(dtp::make_nullval())
		{}

		elem(elem&& a) noexcept(true) :
			val(std::move(a.val))
		{
		}

		void operator=(elem&& a) noexcept(true)
		{
			val = std::move(a.val);
		}

		elem(elem const& a) :
			val(!a.val.is_nullval() ? std::move(a.val->duplicate().val) : dtp::make_nullval())
		{}

		void operator=(elem const& a)
		{
			val = (!a.val.is_nullval() ? std::move(a.val->duplicate().val) : dtp::make_nullval());
		}

		bool operator==(elem const& a) const
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

		bool operator!=(elem const& a) const
		{
			return !operator==(a);
		}


		std::string str() const
		{
			if (val.is_nullval())
			{
				return "#";
			}
			else
			{
				return val->make_string();
			}
		}


		void make_executable(executable& a) const;

		static elem make(literal_value&& a)
		{
			return elem{ dtp::make<literal>(std::move(a)) };
		}

		static elem make(variable_value&& a)
		{
			return elem{ dtp::make<variable>(std::move(a)) };
		}

		static elem make(function&& a)
		{
			return elem{ dtp::make<function>(a) };
		}

		static elem make_empty()
		{
			return elem{ dtp::make_nullval() };
		}

		static elem make(std::string const& to_be_parsed)
		{
			std::string::const_iterator it = to_be_parsed.cbegin();
			std::optional<elem> o{ parse(it,to_be_parsed.cend()) };
			if (!o)
			{
				return make_empty();
			}
			while (it != to_be_parsed.cend())
			{
				if (*it != ' ')
				{
					return make_empty();
				}
				++it;
			}
			return std::move(*o);
		}

		static elem make(char const* to_be_parsed)
		{
			return make(std::string{ to_be_parsed });
		}


		static std::optional<elem> parse(std::string::const_iterator& start, std::string::const_iterator stop)
		{
			if (start == stop)
			{
				return std::nullopt;
			}
			if (*start == '@')
			{
				std::optional<variable> n = variable::parse(start, stop);
				if (n)
				{
					return std::optional<elem>{elem{ dtp::make<variable>(std::move(*n)) }};
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
					return std::optional<elem>{elem{ dtp::make<function>(std::move(*n)) }};
				}
				else
				{
					return std::nullopt;
				}
			}
			else if (*start == '#')
			{
				++start;
				return std::optional<elem>{elem::make_empty()};
			}
			else
			{
				std::optional<literal> n = literal::parse(start, stop);
				if (n)
				{
					return std::optional<elem>{elem{ dtp::make<literal>(std::move(*n)) }};
				}
				else
				{
					return std::nullopt;
				}
			}
		}

		static elem literal_parse(std::string::const_iterator& start, std::string::const_iterator stop)
		{
			auto g = literal::parse(start, stop);
			if (g)
			{
				return elem::make(std::move(g->data));
			}
			else
			{
				return elem::make_empty();
			}
		}

		static elem variable_parse(std::string::const_iterator& start, std::string::const_iterator stop)
		{
			auto g = variable::parse(start, stop);
			if (g)
			{
				return elem::make(std::move(g->data));
			}
			else
			{
				return elem::make_empty();
			}
		}

		static elem function_parse(std::string::const_iterator& start, std::string::const_iterator stop)
		{
			auto g = function::parse(start, stop);
			if (g)
			{
				return elem::make(std::move(g->data));
			}
			else
			{
				return elem::make_empty();
			}
		}


		literal_value* get_literal()
		{
			if (val.is_nullval() && val->is_literal())
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
			if (val.is_nullval() && val->is_variable())
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
			if (val.is_nullval() && val->is_function())
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
			if (val.is_nullval() && val->is_literal())
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
			if (val.is_nullval() && val->is_variable())
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
			if (val.is_nullval() && val->is_function())
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


		elem(dtp&& a) :
			val(std::move(a))
		{}
	};

	const elem empty_elem = elem::make_empty();

	std::ostream& operator<<(std::ostream& stream, elem const& expression)
	{
		stream << expression.str();
		return stream;
	}

	std::istream& operator>>(std::istream& stream, elem& expression)
	{
		std::string to_parse;
		std::getline(stream, to_parse);
		expression = elem::make(to_parse);
		return stream;
	}

	inline std::string function::make_string() const
	{
		std::string ret;
		ret.reserve(data.fn_name.size() + data.arguments.size() * 10);
		ret = data.fn_name;
		ret.push_back('(');
		//for(std::vector<elem>::const_iterator it = arguments.cbegin(); it!=arguments.cend(); ++it)
		//for(int i = 0; i!=arguments.size(); ++i)
		for (auto const& it : data.arguments)
		{
			//ret.append(it->str());
			//ret.append(arguments.at(i).str());
			if (it.val.is_nullval())
			{
				ret.push_back('#');
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

	inline std::optional<function> function::parse(std::string::const_iterator& start, std::string::const_iterator stop)
	{
		assert(start != stop && (name_checker::isupper(*start) || name_checker::islower(*start)));


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

		while (start != stop && *start == ' ')
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

			while (start != stop && *start == ' ')
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

			auto next = elem::parse(start, stop);

			if (next)
			{
				ret.data.arguments.emplace_back(std::move(*next));
			}
			else
			{
				return std::nullopt;
			}

			while (start != stop && *start == ' ')
			{
				++start;
			}

			if (start != stop && *start != ',' && *start != ')')
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
			std::vector<elem> const &r = static_cast<function const *>(a)->data.arguments;
			if (r.size() != data.arguments.size())
			{
				return false;
			}
			for (int i = 0; i != data.arguments.size(); ++i)
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

	inline elem literal::duplicate() const
	{
		return elem{ dtp::make<literal>(std::move(*this)) };
	}

	inline elem variable::duplicate() const
	{
		return elem{ dtp::make<variable>(std::move(*this)) };
	}

	inline elem function::duplicate() const
	{
		return elem{ dtp::make<function>(std::move(*this)) };
	}

}