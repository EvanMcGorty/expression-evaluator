#pragma once

#include<array>
#include<string>
#include<typeinfo>
#include<assert.h>
#include<functional>

namespace mu
{

    template<typename derived,typename base>
    class mover
    {
    public:
        static void move_construct(base* from, base* to)
        {
            new (static_cast<derived*>(to)) derived{std::move(*static_cast<derived*>(from))};
            from->~base();
        }
    };

	template<typename base>
	class null_mover
	{
	public:
		static void move_construct(base* from, base* to)
		{}
	};



#ifdef _DEBUG
std::string allocation_log;
#endif

template<typename base,size_t cap>
//a class that allows usage of dynamically sized types on the stack, given a size cap.
//support for calling virtual methods and casting.
//does not track destructors, uses virtual destructors base has one.
//all types
class stack_virt
{
    template<typename fbase,size_t fcap>
    friend class stack_virt;
public:

    stack_virt()
    {
        static_assert(cap >= sizeof(base), "cap must be larger than the size of the base class");
        set_state_not_nonnull();
    }

    template<typename target_type = base,typename...arg_types>
    static void make(stack_virt<base,cap>& ret, arg_types&&...args)
    {
        static_assert(std::is_base_of<base,target_type>::value || std::is_same<base,target_type>::value,"stack_virt<b,c> can be made with the construction of any class that derives from base, or is base itself");
        if(ret.check_state_whether_nonnull())
        {
            ret.call_destructor_on_data();
        }
        else
        {
            ret.set_state_nonnull();
        }
        ret.call_constructor_on_data<target_type>(std::forward<arg_types>(args)...);
    }

    static void make_nullval(stack_virt<base,cap>& ret)
    {
        if(ret.check_state_whether_nonnull())
        {
            ret.call_destructor_on_data();
        }
        else
        {
            ret.set_state_nonnull();
        }
        ret.set_state_not_nonnull();
    }

    bool is_nullval() const
    {
        return !check_state_whether_nonnull();
    }

    stack_virt(stack_virt const& a) = delete;

    void operator=(stack_virt const& a) = delete;

    template<typename b, size_t c>
    stack_virt(stack_virt<b,c>&& a, std::function<void(base*,base*)> const& move_functor)
    {
        static_assert(cap >= sizeof(base), "cap must be larger than the size of the base class");
        static_assert((std::is_base_of<base,b>::value || std::is_same<base,b>::value) && c<=cap, "to construct stack_virt<xb,xc> from stack_virt<yb,yc>&&, yb must be the same as or derive from xb, and yc must be less than or equal to xc");
		set_state_not_nonnull();
		if(a.check_state_whether_nonnull())
        {
            set_state_nonnull();
            move_functor(a.get(),get());
            a.set_state_not_nonnull();
        }
    }

    template<typename b, size_t c>
    void assign(stack_virt<b,c>&& a, std::function<void(base*,base*)> const& move_functor)
    {
        static_assert((std::is_base_of<base,b>::value || std::is_same<base,b>::value) && c<=cap, "to assign stack_virt<xb,xc>&& to stack_virt<yb,yc>, xb must be the same as or derive from yb, and xc must be less than or equal to yc");
		if (check_state_whether_nonnull())
		{
			call_destructor_on_data();
			set_state_not_nonnull();
		}
		if(a.check_state_whether_nonnull())
        {
            set_state_nonnull();
            move_functor(a.get(),get());
            a.set_state_not_nonnull();
        }
    }

    template<typename newbase>
    void upcast(stack_virt<newbase,cap>& ret, std::function<void(base*,base*)> const& move_functor) &&
    {
        static_assert((std::is_base_of<newbase,base>::value || std::is_same<newbase,base>::value), "to upcast a stack_virt<xb,xc>&& into a stack_virt<yb,yc>, xb must derive from yb");
        if(ret.check_state_whether_nonnull())
        {
            ret.call_destructor_on_data();
        }
        else
        {
            ret.set_state_nonnull();
        }
        if(check_state_whether_nonnull())
        {
            ret.set_state_nonnull();
            set_state_not_nonnull();
        }
        else
        {
            ret.set_state_not_nonnull();
        }
    }

    template<size_t newcap>
    void enlarge(stack_virt<base,newcap>& ret, std::function<void(base*,base*)> const& move_functor) &&
    {
        static_assert(newcap>=cap,"can not enlarge to a smaller capacity");
        if(ret.check_state_whether_nonnull())
        {
            ret.call_destructor_on_data();
        }
        else
        {
            ret.set_state_nonnull();
        }
        if(check_state_whether_nonnull())
        {
            ret.set_state_nonnull();
            move_functor(get(),ret.get());
            set_state_not_nonnull();
        }
        else
        {
            ret.set_state_not_nonnull();
        }
    }

    base* get()
    {
        if(!check_state_whether_nonnull())
        {
            return nullptr;
        }
        else
        {
            return reinterpret_cast<base*>(&data[0]);
        }
    }

    base const* get() const
    {
        if(!check_state_whether_nonnull())
        {
            return nullptr;
        }
        else
        {
            return reinterpret_cast<base const*>(&data[0]);
        }
    }

    base& operator*()
    {
        return *get();
    }

    base const& operator*() const
    {
        return *get();
    }

    base* operator->()
    {
        return get();
    }

    base const* operator->() const
    {
        return get();
    }

    



    template<typename d>
    bool can_downcast() const&
    {
        static_assert((std::is_base_of<base,d>::value || std::is_same<base,d>::value) && sizeof(d) <= cap,"to downcast stack_virt<xb,xc>&& to yb* or stack_virt<yb,xc>, yb must derive from xb and sizeof(yb) must be less than or equal to xc");
        return nullptr != dynamic_cast<d const*>(get());
    }

    template<typename d>
    void downcast(stack_virt<d,cap>& ret, std::function<void(base*,base*)> const& move_functor) &&
    {
        static_assert((std::is_base_of<base,d>::value || std::is_same<base,d>::value) && sizeof(d) <= cap,"to downcast stack_virt<xb,xc>&& to y*, yb must derive from xb and");
        assert(can_downcast<d>());
        if(ret.check_state_whether_nonnull())
        {
            ret.call_destructor_on_data();
        }
        else
        {
            ret.set_state_nonnull();
        }
        ret.set_state_nonnull();
        move_functor(get(),ret.get());
        set_state_not_nonnull();
    }

    template<typename d>
    d* downcast_get()
    {
        static_assert((std::is_base_of<base,d>::value || std::is_same<base,d>::value) && sizeof(d) <= cap,"to downcast stack_virt<xb,xc>&& to y*, yb must derive from xb and");
        assert(can_downcast<d>());
        return static_cast<d*>(get());
    }

    template<typename d>
    d const* downcast_get() const
    {
        static_assert((std::is_base_of<base,d>::value || std::is_same<base,d>::value) && sizeof(d) <= cap,"to downcast stack_virt<xb,xc>&& to y*, yb must derive from xb and");
        assert(can_downcast<d>());
        return static_cast<d const*>(get());
    }

    
    static constexpr size_t view_cap()
    {
        return cap;
    }


    
    

    stack_virt<base,sizeof(base)> shrink(stack_virt<base,sizeof(base)>& ret, std::function<void(base*,base*)> const& move_functor) &&
    {
        assert(is_only_base());

        if(ret.check_state_whether_nonnull())
        {
            ret.call_destructor_on_data();
        }
        else
        {
            ret.set_state_nonnull();
        }

        if(check_state_whether_nonnull())
        {
            ret.set_state_nonnull();
            move_functor(get(),ret.get());
            set_state_not_nonnull();
        }
        else
        {
            ret.set_state_not_nonnull();
        }
    }

    bool is_only_base() const
    {
        return typeid(get()) == typeid(base);
    }

    //trusts user that the dynamically stored type is not bigger than newcap
    template<size_t newcap> 
    void unsafe_set_cap(stack_virt<base,newcap>& ret,std::function<void(base*,base*)> const& move_functor) &&
    {
        if(ret.check_state_whether_nonnull())
        {
            ret.call_destructor_on_data();
        }
        else
        {
            ret.set_state_nonnull();
        }
        if(check_state_whether_nonnull())
        {
            ret.set_state_nonnull();
            move_functor(get(),ret.get());
            set_state_not_nonnull();
        }
        else
        {
            ret.set_state_not_nonnull();
        }
    }

    ~stack_virt()
    {
        if(check_state_whether_nonnull())
        {
            call_destructor_on_data();
			set_state_not_nonnull();
        }
    }

private:

    template<typename target_type, typename...ts>
    void call_constructor_on_data(ts&&...args)
    {
        #ifdef _DEBUG
            allocation_log.append("construction of ");
            allocation_log.append(typeid(target_type).name());
            allocation_log.append(" into a pointer of ");
            allocation_log.append(typeid(base).name());
            allocation_log.push_back('\n');
        #endif
        new (get()) target_type(std::forward<ts>(args)...);
    }

    void call_destructor_on_data()
    {
        #ifdef _DEBUG
            if constexpr(std::has_virtual_destructor<base>::value)
            {
                allocation_log.append("virtual destruction of ");
                allocation_log.append(typeid(*get()).name());
            }
            else
            {
                allocation_log.append("nonvirtual destruction of ");
                allocation_log.append(typeid(*get()).name());
            }
            allocation_log.append(" through base ");
            allocation_log.append(typeid(base).name());
            allocation_log.push_back('\n');
        #endif
        get()->~base();
    }

    void set_state_nonnull()
    {
        is_nonnull = true;
    }

    void set_state_not_nonnull()
    {
        is_nonnull = false;
    }

    bool check_state_whether_nonnull() const
    {
        return is_nonnull;
    }


    

    std::array<unsigned char,cap> data;
    bool is_nonnull; //there needs to be some representation of a null state after an r-value is moved out of
};

}