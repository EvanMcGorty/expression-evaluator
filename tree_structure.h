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

    virtual bool is_call() const
    {
        return false;
    }

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
            ret.push_back('-');
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
            if(!(name_checker::isupper(*start) || name_checker::islower(*start)))
            {
                switch(*start)
                {
                case('+'):
                    sc_ret = sc::push;
                    break;
                case('-'):
                    sc_ret = sc::pop;
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
                return std::optional<variable>{variable{std::move(ret),sc_ret}};
            }
        }

        return std::nullopt;

    }


private:

    sc change;

    std::string var_name;
};


class call : public node
{
public:

    call(name_checker&& n)
    {
        fn_name = std::move(n);
    }

    call(call&& a)
    {
        fn_name = std::move(a.fn_name);
        arguments = std::move(a.arguments);
    }

    bool is_call() const override
    {
        return true;
    }

    std::string make_string() const override;

    static std::optional<call> parse(std::string::const_iterator& start, std::string::const_iterator stop);

private:

    std::string fn_name;

    std::vector<elem> arguments;
};



class elem
{
public:

    elem(mu::algebraic<node,literal,variable,call>&& a) :
        val(std::move(a))
    {

    }

    elem(elem&& a) :
        val(std::move(a.val))
    {

    }


    mu::algebraic<node,literal,variable,call> val;
};

inline std::string call::make_string() const
{
    std::string ret;
    ret.reserve(fn_name.size()+arguments.size()*10);
    ret = fn_name;
    ret.push_back('(');
    for(auto it = arguments.cbegin(); it!=arguments.cend(); ++it)
    {
        ret.append(it->val->make_string());
        ret.push_back(',');
    }
    *(ret.end()-1) = ')';

    return ret;
}

inline std::optional<call> call::parse(std::string::const_iterator& start, std::string::const_iterator stop)
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
    call ret{name_checker{std::move(retname)}};

    while(start!=stop && *start==' ')
    {
        ++start;
    }

    if(start == stop || *start != '(')
    {
        return ret;
    }

    while(start!=stop && *start==' ')
    {
        ++start;
    }

    do
    {

        if(*start == '@')
        {
            std::optional<variable> n = variable::parse(start,stop);
            if(n)
            {
                ret.arguments.emplace_back(mu::algebraic<node,literal,variable,call>::make<variable>(std::move(*n)));
            }
            else
            {
                return std::nullopt;
            }
        }
        else if(name_checker::isupper(*start) || name_checker::islower(*start))
        {
            std::optional<call> n{call::parse(start,stop)};
            if(n)
            {
                ret.arguments.emplace_back(mu::algebraic<node,literal,variable,call>::make<call>(std::move(*n)));
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
                ret.arguments.emplace_back(mu::algebraic<node,literal,variable,call>::make<literal>(std::move(*n)));
            }
            else
            {
                return std::nullopt;
            }
        }

        while(start!=stop && *start==' ')
        {
            ++start;
        }
    }
    while(start!=stop && *start != ')');

    if(start==stop)
    {
        return std::nullopt;
    }

    return ret;

}