#pragma once
#include<type_traits>
#include<utility>
#include<functional>

namespace expr
{
	namespace impl
	{
		
		
		template<typename t>
		class type
		{
		public:

			typedef t held;

			type()
			{}

			static constexpr bool is_raw()
			{
				return true;
			}

			static constexpr bool is_ref()
			{
				return false;
			}

			static constexpr bool is_val()
			{
				return false;
			}

		};


		template<typename t>
		struct ref_wrap
		{
			explicit ref_wrap(t& a) :
				to(&a)
			{
				static_assert(!std::is_reference_v<t>);
				static_assert(!type<t>::is_ref() && !type<t>::is_val() && type<t>::is_raw());
			}

			explicit operator t&()
			{
				return *to;
			}

			ref_wrap() = delete;
			ref_wrap(ref_wrap const&) = default;
			ref_wrap(ref_wrap&&) = default;
			ref_wrap& operator=(ref_wrap const&) = default;
			ref_wrap& operator=(ref_wrap&&) = default;
			~ref_wrap() = default;

			t* to;
		};

		//will wrap a single instance of a class.
		//for classes that cannot be moved but can be copied, this will supply a move constructor that copies.
		//same goes for assignment operators
		template<typename t, typename enable = void>
		struct val_wrap
        {}; //this should never be instanciated

        template<typename t>
        struct val_wrap<t, std::enable_if_t<std::is_move_constructible_v<t> && std::is_move_assignable_v<t>>>
		{
			explicit val_wrap(t&& a) :
				wrapped(std::move(a))
			{
				static_assert(!std::is_reference_v<t> && !std::is_const_v<t>);
				static_assert(!type<t>::is_ref() && !type<t>::is_val() && type<t>::is_raw());
			}

			explicit operator t&&() &&
			{
				return std::move(wrapped);
			}


			val_wrap() = delete;
			val_wrap(val_wrap const&) = default;
			val_wrap(val_wrap&&) = default;
			val_wrap& operator=(val_wrap const&) = default;
			val_wrap& operator=(val_wrap&&) = default;
			~val_wrap() = default;

			t wrapped;

		};

		template <typename t>
		struct val_wrap<t, std::enable_if_t<!std::is_move_constructible_v<t> && std::is_move_assignable_v<t>>>
		{
			explicit val_wrap(t const& a) :
				wrapped(a)
			{
				static_assert(!std::is_reference_v<t> && !std::is_const_v<t>);
				static_assert(!type<t>::is_ref() && !type<t>::is_val() && type<t>::is_raw());
			}

			explicit operator t&&() &&
			{
				return std::move(wrapped);
			}


			val_wrap() = delete;
			val_wrap(val_wrap const& rhs) :
				wrapped(rhs.wrapped)
			{ }
			val_wrap(val_wrap&& rhs) :
				wrapped(rhs.wrapped)
			{ }
			val_wrap& operator=(val_wrap const&) = default;
			val_wrap& operator=(val_wrap&&) = default;
			~val_wrap() = default;

			t wrapped;
		};

        template<typename t>
        struct val_wrap<t,std::enable_if_t<std::is_move_constructible_v<t> && !std::is_move_assignable_v<t>>>
		{
			explicit val_wrap(t&& a) :
				wrapped(std::move(a))
			{
				static_assert(!std::is_reference_v<t> && !std::is_const_v<t>);
				static_assert(!type<t>::is_ref() && !type<t>::is_val() && type<t>::is_raw());
			}

			explicit operator t&&() &&
			{
				return std::move(wrapped);
			}


			val_wrap() = delete;
			val_wrap(val_wrap const&) = default;
			val_wrap(val_wrap&&) = default;
			val_wrap& operator=(val_wrap const& rhs)
            {
                wrapped = rhs.wrapped;
                return *this;
            }
			val_wrap& operator=(val_wrap&& rhs)
            {
                wrapped = rhs.wrapped;
                return *this;
            }
			~val_wrap() = default;

			t wrapped;

		};

		template <typename t>
		struct val_wrap<t, std::enable_if_t<!std::is_move_constructible_v<t> && !std::is_move_assignable_v<t>>>
		{
			explicit val_wrap(t const& a) :
				wrapped(a)
			{
				static_assert(!std::is_reference_v<t> && !std::is_const_v<t>);
				static_assert(!type<t>::is_ref() && !type<t>::is_val() && type<t>::is_raw());
			}

			explicit operator t&&() &&
			{
				return std::move(wrapped);
			}


			val_wrap() = delete;
			val_wrap(val_wrap const& rhs) :
				wrapped(rhs.wrapped)
			{ }
			val_wrap(val_wrap&& rhs) :
				wrapped(rhs.wrapped)
			{ }
			val_wrap& operator=(val_wrap const& rhs)
            {
                wrapped = rhs.wrapped;
                return *this;
            }
			val_wrap& operator=(val_wrap&& rhs)
            {
                wrapped = rhs.wrapped;
                return *this;
            }
			~val_wrap() = default;

			t wrapped;
		};

		template<typename t>
		class type<val_wrap<t>>
		{
		public:

			typedef val_wrap<t> held;

			typedef t raw;

			typedef t&& will_pass;

			type()
			{}
			
			static constexpr bool is_raw()
			{
				return false;
			}
			
			static constexpr bool is_ref()
			{
				return false;
			}

			static constexpr bool is_val()
			{
				return true;
			}
		};

		
		template<typename t>
		class type<ref_wrap<t>>
		{
		public:

			typedef ref_wrap<t> held;

			typedef t raw;

			typedef t& will_pass;

			type()
			{}
			
			static constexpr bool is_raw()
			{
				return false;
			}
			
			static constexpr bool is_ref()
			{
				return true;
			}

			static constexpr bool is_val()
			{
				return false;
			}
		};

		//if t is what you see in a function argument, pre_call_t<t> is what is held before it is passed.
		//or, at least what is held should be implicitly convertible (by the expression evaluator) into pre_call_t<t>
		template<typename t>
		constexpr auto as_storable()
		{
			if constexpr (std::template is_rvalue_reference_v<t>)
			{
				if constexpr (std::is_const_v<typename std::remove_reference_t<t>>)
				{
					return type<val_wrap<typename std::remove_reference_t<t>>>();
				}
				else
				{
					return type<val_wrap<typename std::remove_reference_t<t>>>();
				}
			}
			else if constexpr (std::template is_lvalue_reference_v<t>)
			{
				if constexpr (std::is_const_v<typename std::remove_reference_t<t>>)
				{
					return type<ref_wrap<typename std::remove_reference_t<t> const>>();
				}
				else
				{
					return type<ref_wrap<typename std::remove_reference_t<t>>>();
				}
			}
			else
			{
				return type<val_wrap<t>>();
			}
		}

		//if t is the argument of the function, at_call_site_t is the type that must be forwarded into the function at the call site
		template<typename t>
		constexpr auto as_passable()
		{
			if constexpr (std::template is_rvalue_reference_v<t>)
			{
				return type<std::remove_reference_t<t>>();
			}
			else if constexpr(std::template is_lvalue_reference_v<t>)
			{
				return type<t>();
			}
			else
			{
				return type<std::remove_const_t<t>&&>();
			}
		}

		//if t is the return type of a function, return_t<t> is what the evaluator stores from the result.
		template<typename t>
		constexpr auto as_returnable()
		{
			if constexpr (std::template is_lvalue_reference_v<t>)
			{
				return type<ref_wrap<typename std::remove_reference_t<t>>>();
			}
			else
			{
				if constexpr (
					std::is_same_v<void,t>
					||
					(
					std::template is_const_v<std::remove_reference_t<t>>
					&&
				 	!std::template is_copy_constructible_v<std::remove_const_t<std::remove_reference_t<t>>>)
					)
				{
					return type<void>();
				}
				else
				{
					return type<val_wrap<typename std::remove_const_t<std::remove_reference_t<t>>>>();
				}
			}
		}

		//msvc 17 needs this goofy workaround in order to compile.

		template<typename t>
		struct pre_call
		{
			typedef decltype(as_storable<t>()) type;
		};

		template<typename t>
		using pre_call_t = typename pre_call<t>::type::held;


		template<typename t>
		struct at_call_site
		{
			typedef decltype(as_passable<t>()) type;
		};

		template<typename t>
		using at_call_site_t = typename at_call_site<t>::type::held;

		template<typename t>
		struct post_return
		{
			typedef decltype(as_returnable<t>()) type;
		};

		template<typename t>
		using post_return_t = typename post_return<t>::type::held;


		//where t is the argument type in the function
		template<typename t>
		constexpr at_call_site_t<t> storable_into_passable(pre_call_t<t>&& x)
		{
			return std::forward<at_call_site_t<t>>(at_call_site_t<t>(std::move(x)));
		}

		template<typename t>
		constexpr bool can_return()
		{
			return !std::is_same_v<void,post_return_t<t>>;
		}

		template<typename t>
		constexpr post_return_t<t> into_returnable(t&& x)
		{
			if constexpr (!can_return<t>())
			{
				return;
			}
			else if constexpr(std::is_lvalue_reference_v<t>)
			{
				return post_return_t<t>(x);
			}
			else
			{
				return post_return_t<t>(std::move(x));
			}
		}



		template<typename ret_t, typename...argts>
		std::function<post_return_t<ret_t>(pre_call_t<argts>...)> make_storable_call(std::function<ret_t(argts...)>&& f)
		{
			if constexpr(can_return<ret_t>())
			{
				return std::function<post_return_t<ret_t>(pre_call_t<argts>&&...)> {
					[f = std::move(f)](pre_call_t<argts>...argvs)->post_return_t<ret_t>
					{
						return into_returnable<ret_t>(f(storable_into_passable<argts>(std::move(argvs))...));
					}
				};
			}
			else
			{
				return std::function<void(pre_call_t<argts>&&...)> {
					[f = std::move(f)](pre_call_t<argts>&&...argvs) -> void
					{
						f(storable_into_passable<argts>(std::move(argvs))...);
					}
				};
			}
		}
	}
}
