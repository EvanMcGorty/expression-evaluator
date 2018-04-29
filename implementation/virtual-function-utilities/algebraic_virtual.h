#pragma once

#include<array>
#include<typeinfo>
#include"stack-based_virtual.h"

namespace mu
{

namespace variadic_utilities
{



template <typename target, typename f, typename...r>
constexpr bool is_one_of()
{
    if constexpr (sizeof...(r) != 0)
    {
        return std::is_same<target, f>::value || is_one_of<target, r...>();
    }
    else
    {
        return std::is_same<target, f>::value;
    }
}

template <typename target, typename f, typename... r>
constexpr bool do_all_derive()
{
    if constexpr (sizeof...(r) != 0)
    {
        return std::is_base_of<target, f>::value && do_all_derive<target, r...>();
    }
    else
    {
        return std::is_base_of<target, f>::value;
    }
}

template <typename f, typename... r>
constexpr size_t largest_class()
{
    if constexpr (sizeof...(r) != 0)
    {
        constexpr size_t rs = largest_class<r...>();
        if constexpr (sizeof(f) > rs)
        {
            return sizeof(f);
        }
        else
        {
            return rs;
        }
    }
    else
    {
        return sizeof(f);
    }
}

template<typename subset_first, typename...subset>
struct is_subset
{

	template<typename...superset>
	static constexpr bool of()
	{

		return is_one_of<subset_first, superset...>() && is_subset<subset...>::template of<superset...>();

	}
};

template<typename subset_only>
struct is_subset<subset_only>
{
	template<typename...superset>
	static constexpr bool of()
	{
		return is_one_of<subset_only, superset...>();
	}
};

template<typename...list>
struct variadic
{
    template<typename f>
    static constexpr variadic<f,list...> push_front()
    {
        return variadic<f,list...>{};
    }
};

template<typename base, typename t, typename...ts>
struct filter
{
    static constexpr auto get()
    {
        if constexpr(sizeof...(ts) == 0)
        {
            if constexpr(std::is_base_of<base,t>())
            {
                return variadic<t>{};
            }
            else
            {
                return variadic<>{};
            }
        }
        else
        {
            auto restp = filter<base,ts...>::get();
            if constexpr(std::is_base_of<base,t>())
            {
                return decltype(decltype(restp)::template push_front<t>()){};
            }
            else
            {
                return restp;
            }
        }
    }
};

}


template<typename base, typename...derived>
class algebraic
{
    template<typename obase,typename...oderived>
    friend class algebraic;

public:

    using dt = stack_virt<base, variadic_utilities::largest_class<derived...>()>;

    template<typename target,typename...arg_types>
    static algebraic<base,derived...> make(arg_types&&...args)
    {
        static_assert(variadic_utilities::is_one_of<target,derived...>(),"target type must be one of the listed derived classes");
        dt ret;
        dt::template make<target>(ret,std::forward<arg_types>(args)...);
        return algebraic{std::move(ret)};
    }

    static algebraic<base,derived...> make_nullval()
    {
        dt ret;
        dt::make_nullval(ret);
        return algebraic{std::move(ret)};
    }

    bool is_nullval() const
    {
        return data.is_nullval();
    }

    algebraic(algebraic const& a) = delete;
    
    void operator=(algebraic const& a) = delete;

    template<typename oldbase,typename...oldderived>
    algebraic(algebraic<oldbase,oldderived...>&& a) noexcept(true) :
        algebraic(dt(std::move(a.data), a.is_nullval() ? &null_mover<oldbase>::move_construct : a.move_functor()))
    {
        perform_fundamental_static_assertions();
        static_assert(variadic_utilities::is_subset<oldderived...>::template of<derived...>(),"cannot construct an algebraic<ts...> from an algebraic that could contain a type other than the ones in ts...");
    }


    template<typename oldbase,typename...oldderived>
    void operator=(algebraic<oldbase,oldderived...>&& a) noexcept(true)
    {
        static_assert(variadic_utilities::is_subset<oldderived...>::template of<derived...>(),"cannot construct an algebraic<ts...> from an algebraic that could contain a type other than the ones in ts...");
        data.assign(std::move(a.data), a.is_nullval() ? &null_mover<oldbase>::move_construct : a.move_functor());
    }


    ~algebraic() noexcept(true)
    {}

    base* get()
    {
        return data.get();
    }

    base const* get() const
    {
        return data.get();
    }

    template<typename d>
    bool can_downcast() const
    {
        static_assert(variadic_utilities::is_one_of<d,derived...>(),"can only downcast to one of the derived types listed as a template parameter");
        return data.template can_downcast<d>();
    }

    template<typename d>
    auto downcast() &&
    {
        static_assert(variadic_utilities::is_one_of<d,derived...>(),"can only downcast to one of the derived types listed as a template parameter");
        assert(data.template can_downcast<d>());
        return changer<d>::change(std::move(*this),variadic_utilities::filter<d,derived...>::get());
    }

    template<typename d>
    d* downcast_get()
    {
        static_assert(variadic_utilities::is_one_of<d,derived...>(),"can only downcast to one of the derived types listed as a template parameter");
        return data.template downcast_get<d>(); //stack_virt already asserts can_downcast<d>
    }

    template<typename d>
    d const* downcast_get() const
    {
        static_assert(variadic_utilities::is_one_of<d,derived...>(),"can only downcast to one of the derived types listed as a template parameter");
        return data.template downcast_get<d>(); //stack_virt already asserts can_downcast<d>
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

    
    static algebraic<base,derived...> unsafe_from_stack_virt(dt&& a)
    {
		if (a.is_nullval())
		{
			return algebraic<base, derived...>::make_nullval();
		}
		else
		{
			algebraic<base, derived...> ret{ dt(a,iterate_find_move_functor<derived...>(typeid(*a.get()))) };
			return ret;
		}
    }

    dt release() &&
    {
        return std::move(data);
    }

    std::function<void(base*,base*)> move_functor() const
    {
        return iterate_find_move_functor<derived...>(typeid(*get()));
    }

private:

    template<typename t, typename...ts>
    std::function<void(base*,base*)> iterate_find_move_functor(std::type_info const& cur_type) const
    {
        if(cur_type == typeid(t))
        {
            return &mover<t,base>::move_construct;
        }
        else
        {
            if constexpr(sizeof...(ts) == 0)
            {
                abort();
            }
            else
            {
                return iterate_find_move_functor<ts...>(cur_type);
            }
        }
    }

    static constexpr void perform_fundamental_static_assertions()
    {
        static_assert(variadic_utilities::do_all_derive<base,derived...>(),"all of derived must derive from base");
    }

    template<typename newbase>
    struct changer
    {
        //doesn't explicitly do any extra static asserts
        template<typename...ts>
        static algebraic<newbase,ts...> change(algebraic&& self, variadic_utilities::variadic<ts...> to)
        {
			auto all_move_functor = self.move_functor();
            stack_virt<base,algebraic<newbase,ts...>::dt::view_cap()> r1;
            std::move(self.data).template unsafe_set_cap<algebraic<newbase,ts...>::dt::view_cap()>(r1,all_move_functor);
            stack_virt<newbase,algebraic<newbase,ts...>::dt::view_cap()> r2;
            std::move(r1).template downcast<newbase>(r2, all_move_functor);
            return algebraic<newbase,ts...>{std::move(r2)};
        }
    };


    algebraic(dt&& a) :
        data((std::move(a)),a.is_nullval() ? &null_mover<base>::move_construct : iterate_find_move_functor<derived...>(typeid(*a.get())))
    {
        perform_fundamental_static_assertions();
    }

    void operator=(dt&& a)
    {
        data = std::move(a);
    }

    dt data;



};

}