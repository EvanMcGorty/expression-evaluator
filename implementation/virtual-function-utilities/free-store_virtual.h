#pragma once

#include<iostream>
#include<memory>
#include<vector>

namespace mu
{

//alternative to unique_ptr<t>
//it owns its data so that when it is const, the data it points to is const.
//is never nullptr until it is moved out of or set the value of virt<t>::make_nullval()
template<typename t>
class virt
{

template<typename ot>
friend class virt;

public:

    //like make_unique
    template<typename target_type = t,typename...ts>
    static virt<t> make(ts&&...args)
    {
        static_assert(std::is_base_of<t,target_type>::value || std::is_same<t,target_type>::value,"virt<t> can be made with the construction of any class that derives from t, or is t itself");
        return virt<t>{std::unique_ptr<t>{std::make_unique<target_type>(std::forward<ts>(args)...)}};
    }

    static virt<t> make_nullval()
    {
        virt<t> ret;
        ret.data = nullptr;
        return ret;
    }

    bool is_nullval() const
    {
        return data == nullptr;
    }

    //compatibility with unique_ptrs
    //may not be a nullptr
    virt(std::unique_ptr<t>&& a)
    {
        data = std::move(a);
    }

    //compatibility with unique_ptrs
    std::unique_ptr<t> release() &&
    {
        return std::move(data);
    }

    template<typename d>
    //move construction and implicit upcast construction
    virt(virt<d>&& a) noexcept(true) :
        data(static_cast<d*>(a.data.release()))
    {
        static_assert(std::is_base_of<t,d>::value || std::is_same<t,d>::value,"to construct virt<x> from virt<y>&&, x must be the same as or derive from y");
    }

    template<typename d>
    //move assignment and implicit upcast assignment
    void operator=(virt<d>&& a) noexcept(true)
    {
        static_assert(std::is_base_of<t,d>::value || std::is_same<t,d>::value,"to assign to virt<x> from virt<y>&&, x must be the same as or derive from y");
        data.reset(std::move(static_cast<d*>(a.data.release())));
    }

    template<typename d>
    virt(virt<d> const& a) = delete;

    template<typename d>
    void operator=(virt<d> const& a) = delete;
    
    virt(virt const& a) = delete;

    void operator=(virt const& a) = delete;


    //like implicit upcasting
    template<typename b>
    virt<b> upcast() &&
    {
        static_assert(std::is_base_of<b,t>::value || std::is_same<b,t>::value,"to upcast virt<x>&& to virt<y>, x must derive from y");
        return virt<b>{std::unique_ptr<b>{static_cast<b*>(data.release())}};
    }
    
    //like trying a dynamic cast
    template<typename d>
    bool can_downcast() const
    {
        static_assert(std::is_base_of<t,d>::value || std::is_same<d,t>::value,"to downcast x to y, y must derive from x");
        return dynamic_cast<d const*>(data.get()) != nullptr;
    }
    
    //like doing a static cast
    template<typename d>
    virt<d> downcast() &&
    {
        static_assert(std::is_base_of<t,d>::value || std::is_same<d,t>::value,"to downcast virt<x>&& to virt<y>, y must derive from x");
        assert(can_downcast<d>());
        return virt<d>{std::unique_ptr<d>{static_cast<d*>(data.release())}};
    }

    //like doing a static cast
    template<typename d>
    d* downcast_get()
    {
        static_assert(std::is_base_of<t,d>::value || std::is_same<d,t>::value,"to downcast_get virt<x> const& as y*, y must derive from x");
        assert(can_downcast<d>());
        return static_cast<d*>(data.get());
    }

    //like doing a static cast
    template<typename d>
    d const* downcast_get() const
    {
        static_assert(std::is_base_of<t,d>::value || std::is_same<d,t>::value,"to downcast_get virt<x> const& as y*, y must derive from x");
        assert(can_downcast<d>());
        return static_cast<d const*>(data.get());
    }

    //data access
    t& operator*()
    {
        return *get();
    }

    //data access
    t const& operator*() const
    {
        return *get();
    }

    //member access
    t* operator->()
    {
        return get();
    }

    //member access
    t const* operator->() const
    {
        return get();
    }

    //write access pointer to data. Does not release ownership of data.
    t* get()
    {
        return data.get();
    }

    //write access pointer to data. Does not release ownership of data.
    t const* get() const
    {
        return data.get();
    }


private:

    virt()
    { }


    //wrapped data
    std::unique_ptr<t> data;
};

}