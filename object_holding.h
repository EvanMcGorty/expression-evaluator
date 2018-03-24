#pragma once

#include"type_asker.h"

#include"statement.h"


class any_elem_val
{
public:
    virtual bool is_object() const
    {
        return false;
    }

    virtual bool is_unparsed() const
    {
        return false;
    }

    virtual bool is_reference() const
    {
        return false;
    }

    //returns true if this is an object and the held value was moved.
    virtual bool get(any_type_ask* tar) = 0;

    virtual ~any_elem_val()
    {

    }
};


//on a stack, this is what a literal pushes.
class unparsed : public any_elem_val
{
public:
    bool is_unparsed() const override
    {
        return true;
    }

    bool get(any_type_ask* tar) override
    {
        tar->parse(value);
        return false;
    }

    std::string value;
};


//the object held by a variable
//on a stack, this is what a function pushes.
class any_object : public any_elem_val
{
public:
    virtual std::type_info const& get_type() const = 0;


    template<typename t>
    bool is() const
    {
        return typeid(t) == get_type();
    }

    bool is_object() const final override
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

    bool get(any_type_ask* tar) override
    {
        if(tar->get_type() == typeid(t*))
        {
            static_cast<type_ask_of<t*>*>(tar)->gotten = &val;
            return false;
        }
        else if(tar->get_type() == typeid(t const*))
        {
            static_cast<type_ask_of<t const*>*>(tar)->gotten = &val;
            return false;
        }
        else if(tar->get_type() == typeid(t))
        {
            static_cast<type_ask_of<t>*>(tar)->gotten = std::move(val);
            return true;
        }
        else
        {
            return false;
        }
    }


    t val;
};



//a variable.
//on a stack, this is what a variable pushes (unless if the variable is also being popped).
class value_reference : public any_elem_val
{
public:

    bool is_reference() const override
    {
        return true;
    }

    bool get(any_type_ask* tar) override
    {
        if(!ref->is_nullval())
        {
            if((**ref).get(tar))
            {
                *ref = mu::virt<any_object>::make_nullval();
            }
        }

        return false;
    }


    mu::virt<any_object>* ref;
};