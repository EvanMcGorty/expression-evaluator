#pragma once

#include"type_asker.h"

#include"statement.h"


class any_elem_val
{
    virtual bool is_object()
    {
        return false;
    }

    virtual bool is_literal()
    {
        return false;
    }

    virtual void get(any_type_ask* tar) = 0;
};


class any_object : public any_elem_val
{
public:
    virtual std::type_info const& get_type() const = 0;

    template<typename t>
    bool is() const
    {
        return typeid(t) == get_type();
    }

    bool is_object() final override
    {
        return true;
    }
};

//instead of object_of<void> use object_of<empty_object>
class empty_object
{

};

template<typename t>
class object_of final : public any_object
{
private:
    constexpr std::type_info const& type() const
    {
        return typeid(t);
    }

public:

    std::type_info const& get_type() const override
    {
        return type();
    }

    void get(any_type_ask* tar) override
    {
        if(tar->get_type == typeid(t))
        {
            static_cast<type_ask_of<t>*>(tar)->gotten = &val;
        }
    }

    t val;
};

