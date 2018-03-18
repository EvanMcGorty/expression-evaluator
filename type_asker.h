#include"literal_conversions.h"
#include<typeinfo>

class any_type_ask
{
public:
    virtual void parse(std::string const& a) = 0;

    virtual std::type_info get_type() const = 0;
};

template<typename t>
class type_ask_of : public any_type_ask
{
public:

    void parse(std::string const& a) override
    {
        gotten = convert<t>(a);
    }

    std::type_info get_type() const
    {
        return typeid(t);
    }

    std::optional<t*> gotten;
};