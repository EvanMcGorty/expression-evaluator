#include"statement_evaluations.h"


class expr
{
	friend class sub_expr;
	friend class const_sub_expr;
public:

	expr(elem&& a) :
		head(std::move(a))
	{}


	operator elem() &&
	{
		return std::move(head);
	}

	operator elem&()
	{
		return head;
	}

	operator elem const&() const
	{
		return head;
	}


	expr() :
		head(elem::make_nullval())
	{}

	expr(expr&& a) noexcept(true) :
		head(std::move(a.head))
	{}

	expr(expr const& a) :
		head(a.head)
	{}


	void operator=(expr&& a) noexcept(true)
	{
		head = std::move(a.head);
	}

	void operator=(expr const& a)
	{
		head = a.head;
	}

	bool operator==(expr const& a) const
	{
		if (head.val.is_nullval())
		{
			if (a.head.val.is_nullval())
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
			if (a.head.val.is_nullval())
			{
				return false;
			}
			else
			{
				return head == a.head;
			}
		}
	}

	bool operator!=(expr const& a) const
	{
		if (head.val.is_nullval())
		{
			if (a.head.val.is_nullval())
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			if (a.head.val.is_nullval())
			{
				return true;
			}
			else
			{
				return head != a.head;
			}
		}
	}


	explicit expr(std::string const& to_parse) :
		head(elem::make(to_parse))
	{
		head = elem::make(to_parse);
	}

	explicit expr(char const* to_parse) :
		head(elem::make(to_parse))
	{
		head = elem::make(to_parse);
	}


	static expr empty()
	{
		return expr{ elem::make_nullval() };
	}

	static expr literal(std::string&& a)
	{
		return expr{ elem::make(::literal{std::move(a)}) };
	}

	static expr variable(name_checker&& a, sc b = sc::neutral)
	{
		return expr{ elem::make(::variable{std::move(a),b}) };
	}

	static expr function(name_checker&& n, std::vector<elem>&& a = {})
	{
		return expr{ elem::make(::function{ std::move(n), std::move(a) }) };
	}


	static expr literal_parse(std::string::const_iterator& start, std::string::const_iterator stop)
	{
		auto g = ::literal::parse(start, stop);
		if (g)
		{
			return expr{ elem::make(std::move(*g)) };
		}
		else
		{
			return expr::empty();
		}
	}

	static expr variable_parse(std::string::const_iterator& start, std::string::const_iterator stop)
	{
		auto g = ::variable::parse(start, stop);
		if (g)
		{
			return expr{ elem::make(std::move(*g)) };
		}
		else
		{
			return expr::empty();
		}
	}

	static expr function_parse(std::string::const_iterator& start, std::string::const_iterator stop)
	{
		auto g = ::function::parse(start, stop);
		if (g)
		{
			return expr{ elem::make(std::move(*g)) };
		}
		else
		{
			return expr::empty();
		}
	}

	static expr parse(std::string::const_iterator& start, std::string::const_iterator stop)
	{
		auto g = elem::parse(start, stop);
		if (g)
		{
			return expr{ std::move(*g) };
		}
		else
		{
			return expr::empty();
		}
	}


	::literal_data* get_literal()
	{
		if (head.val->is_literal())
		{
			return &head.val.downcast_get<::literal>()->data;
		}
		else
		{
			return nullptr;
		}
	}

	::variable_data* get_variable()
	{
		if (head.val->is_variable())
		{
			return &head.val.downcast_get<::variable>()->data;
		}
		else
		{
			return nullptr;
		}
	}

	::function_data* get_function()
	{
		if (head.val->is_function())
		{
			return &head.val.downcast_get<::function>()->data;
		}
		else
		{
			return nullptr;
		}
	}


	::literal_data const* get_literal() const
	{
		if (head.val->is_literal())
		{
			return &head.val.downcast_get<::literal>()->data;
		}
		else
		{
			return nullptr;
		}
	}

	::variable_data const* get_variable() const
	{
		if (head.val->is_variable())
		{
			return &head.val.downcast_get<::variable>()->data;
		}
		else
		{
			return nullptr;
		}
	}

	::function_data const* get_function() const
	{
		if (head.val->is_function())
		{
			return &head.val.downcast_get<::function>()->data;
		}
		else
		{
			return nullptr;
		}
	}


	friend std::ostream& operator<<(std::ostream& stream, expr const& expression);

	friend std::istream& operator>>(std::istream& stream, expr& expression);

private:

	elem head;
};


std::ostream& operator<<(std::ostream& stream, expr const& expression)
{
	stream << expression.head.str();
	return stream;
}

std::istream& operator>>(std::istream& stream, expr& expression)
{
	std::string to_parse;
	std::getline(stream, to_parse);
	expression = expr{ to_parse };
	return stream;
}