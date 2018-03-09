#include<vector>
#include<optional>

#include"virtual-function-utilities\algebraic_virtual.h"

#include"parsing_checks.h"


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


class literal : public node
{
public:

    literal(std::string&& a)
    {
        value = a;
    }


    bool is_literal() const override
    {
        return true;
    }


    elem duplicate() const override;


    bool is_equal(node const* a) const override
    {
        return a->is_literal() && static_cast<literal const*>(a)->value == value;
    }


    std::string make_string() const override
    {
        std::string ret("\"");
        ret.reserve(size_t(float(value.length()) * 1.1) + 10);
        bool in_text = true;
        for (auto it = value.cbegin(); it != value.cend(); ++it)
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
        
        if(*start != '"')
        {
            while(start!=stop && !name_checker::canbeafterelem(*start))
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


private:


    std::string value;
};

class variable : public node
{
public:

    enum class sc
    {
        pop,
        neutral,
        push
    };

    variable(name_checker&& a, sc b)
    {
        change = b;
        var_name = std::string(std::move(a));
    }



    bool is_variable() const override
    {
        return true;
    }

    elem duplicate() const override;


    bool is_equal(node const* a) const override
    {
        return a->is_variable() && static_cast<variable const*>(a)->change == change && static_cast<variable const*>(a)->var_name == var_name;
    }


    std::string make_string() const override
    {
        std::string ret;

        switch(change)
        {
        case(sc::neutral):
            ret = "@" + var_name;
            break;
        case(sc::pop):
            ret.reserve(var_name.size()+2);
            ret.push_back('@');
            ret.append(var_name);
            ret.push_back('~');
            break;
        case(sc::push):
            ret.reserve(var_name.size()+2);
            ret.push_back('@');
            ret.append(var_name);
            ret.push_back('+');
            break;
        }
        return ret;
    }


    static std::optional<variable> parse(std::string::const_iterator& start, std::string::const_iterator stop)
    {
        assert(start!=stop && *start == '@');

        std::string ret;
        sc sc_ret;

        ++start;
        if(start==stop || !(name_checker::isupper(*start) || name_checker::islower(*start)))
        {
            return std::nullopt;
        }

        while(start!=stop)
        {
            if(!(name_checker::isinnerchar(*start)))
            {
                switch(*start)
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
                if(ret.size()==0)
                {
                    return std::nullopt;
                }
                else
                {
                    return std::optional<variable>{variable{std::move(ret),sc_ret}};
                }
            }
            ret.push_back(*start);
            ++start;
        }

        if(ret.size()==0 || name_checker::isotherinner(ret[ret.size()-1]))
        {
            return std::nullopt;
        }
        else
        {
            return std::optional<variable>{variable{std::move(ret),sc::neutral}};
        }
    }


private:

    sc change;

    std::string var_name;
};


class function : public node
{
public:

    function(name_checker&& n, std::vector<elem> a = {})
    {
        fn_name = std::move(n);
    }



    bool is_function() const override
    {
        return true;
    }

    elem duplicate() const override;


    bool is_equal(node const* a) const override;


    std::string make_string() const override;

    static std::optional<function> parse(std::string::const_iterator& start, std::string::const_iterator stop);

private:

    std::string fn_name;

    std::vector<elem> arguments;
};


using dtp = mu::algebraic<node,literal,variable,function>;

class elem
{
    friend elem literal::duplicate() const;
    friend elem variable::duplicate() const;
    friend elem function::duplicate() const;
public:

    elem(elem&& a) :
        val(std::move(a.val))
    {

    }

    void operator=(elem&& a)
    {
        val = std::move(a.val);
    }

    elem(elem const& a) :
        elem(a.val->duplicate())
    {

    }

    void operator=(elem const& a)
    {
        operator=(a.val->duplicate());
    }

    bool operator==(elem const& a) const
    {
        return val->is_equal(&*a.val);
    }

    bool operator!=(elem const& a) const
    {
        return !val->is_equal(&*a.val);
    }

    std::string str() const
    {
        return val->make_string();
    }

    static elem make(literal&& a)
    {
        return elem{dtp::make<literal>(a)};
    }

    static elem make(variable&& a)
    {
        return elem{dtp::make<variable>(a)};
    }

    static elem make(function&& a)
    {
        return elem{dtp::make<function>(a)};
    }

    static elem make_nullval()
    {
        return elem{dtp::make_nullval()};
    }

    static elem make(std::string const& to_be_parsed)
    {
        std::string::const_iterator it = to_be_parsed.cbegin();
        std::optional<elem> o {parse(it,to_be_parsed.cend())};
        if(!o)
        {
            return make_nullval();
        }
        while(it!=to_be_parsed.cend())
        {
            if(*it != ' ')
            {
                return make_nullval();
            }
            ++it;
        }
        return std::move(*o);
    }

    static elem make(char const* to_be_parsed)
    {
        return make(std::string{to_be_parsed});
    }

    static std::optional<elem> parse(std::string::const_iterator& start, std::string::const_iterator stop)
    {
        if(*start == '@')
        {
            std::optional<variable> n = variable::parse(start,stop);
            if(n)
            {
                return std::optional<elem>{elem{dtp::make<variable>(std::move(*n))}};
            }
            else
            {
                return std::nullopt;
            }
        }
        else if(name_checker::isupper(*start) || name_checker::islower(*start))
        {
            std::optional<function> n{function::parse(start,stop)};
            if(n)
            {
                return std::optional<elem>{elem{dtp::make<function>(std::move(*n))}};
            }
            else
            {
                return std::nullopt;
            }
        }
        else
        {
            std::optional<literal> n = literal::parse(start,stop);
            if(n)
            {
                return std::optional<elem>{elem{dtp::make<literal>(std::move(*n))}};
            }
            else
            {
                return std::nullopt;
            }
        }
    }

private:
    dtp val;
    

    elem(dtp&& a) :
        val(std::move(a))
    {

    }
};

inline std::string function::make_string() const
{
    std::string ret;
    ret.reserve(fn_name.size()+arguments.size()*10);
    ret = fn_name;
    ret.push_back('(');
    //for(std::vector<elem>::const_iterator it = arguments.cbegin(); it!=arguments.cend(); ++it)
    for(int i = 0; i!=arguments.size(); ++i)
    {
        //ret.append(it->str());
        ret.append(arguments[i].str());
        ret.push_back(',');
    }
    if(arguments.size()>0)
    {
        ret.pop_back();
    }

    ret.push_back(')');

    return ret;
}

inline std::optional<function> function::parse(std::string::const_iterator& start, std::string::const_iterator stop)
{
    assert(start!=stop && (name_checker::isupper(*start)||name_checker::islower(*start)));
    

    std::string retname;
    while(start!=stop && name_checker::isinnerchar(*start))
    {
        retname.push_back(*start);
        ++start;
    }
    if(!name_checker::isendchar(*(retname.end()-1)))
    {
        return std::nullopt;
    }
    function ret{name_checker{std::move(retname)}};

    while(start!=stop && *start==' ')
    {
        ++start;
    }

    if(start == stop || *start != '(')
    {
        return std::optional<function>{std::move(ret)};
    }



    while(start!=stop && *start != ')')
    {
        ++start;

        while(start!=stop && *start==' ')
        {
            ++start;
        }

        if(start==stop)
        {
            return std::nullopt;
        }

        if(*start == ')')
        {
            ++start;
            return std::optional<function>{std::move(ret)};
        }

        auto next = elem::parse(start,stop);

        if(next)
        {
            ret.arguments.emplace_back(std::move(*next));
        }
        else
        {
            return std::nullopt;
        }

        while(start!=stop && *start==' ')
        {
            ++start;
        }

        if(start!=stop && *start != ',' && *start != ')')
        {
            return std::nullopt;
        }


    }

    if(start==stop)
    {
        return std::nullopt;
    }

    ++start;

    return std::optional<function>{std::move(ret)};

}

bool function::is_equal(node const *a) const
{
    if (a->is_function() && static_cast<function const *>(a)->fn_name == fn_name)
    {
        std::vector<elem> const &r = static_cast<function const *>(a)->arguments;
        if(r.size() != arguments.size())
        {
            return false;
        }
        for(int i = 0; i!= arguments.size(); ++i)
        {
            if(r[i] != arguments[i])
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

elem literal::duplicate() const
{
    return elem{dtp::make<literal>(std::move(*this))};
}

elem variable::duplicate() const
{
    return elem{dtp::make<variable>(std::move(*this))};
}

elem function::duplicate() const
{
    return elem{dtp::make<function>(std::move(*this))};
}