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
				static_assert(std::is_trivial_v<ref_wrap<t>>);
				static_assert(!type<t>::is_ref() && !type<t>::is_val() && type<t>::is_raw());
			}

			explicit operator t&()
			{
				return *to;
			}

			ref_wrap() = delete;
			ref_wrap(ref_wrap const&) = default;
			ref_wrap(ref_wrap&&) = default;
			~ref_wrap() = default;

			t* to;
		};

		template<typename t>
		struct val_wrap
		{
			explicit val_wrap(t&& a) :
				wrapped(std::move(a))
			{
				static_assert(!std::is_reference_v<t> && !std::is_const_v<t>);
				static_assert(std::is_trivial_v<val_wrap<t>> == std::is_trivial_v<t>);
				static_assert(!type<t>::is_ref() && !type<t>::is_val() && type<t>::is_raw());
			}

			explicit operator t() &&
			{
				return std::move(wrapped);
			}


			val_wrap() = delete;
			val_wrap(val_wrap const&) = default;
			val_wrap(val_wrap&&) = default;
			~val_wrap() = default;

			t wrapped;

		};

		template<typename t>
		class type<val_wrap<t>>
		{
		public:

			typedef val_wrap<t> held;

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

		//if t is what you see in a function argument, store_t<t> is what is held before it is passed.
		template<typename t>
		constexpr auto as_storable()
		{
			if constexpr (std::template is_rvalue_reference_v<t>)
			{
				if constexpr (std::is_const_v<typename std::remove_reference_t<t>>)
				{
					return type<ref_wrap<typename std::remove_reference_t<t> const>>();
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

		//if t is the argument of the function, pass_t is the type that must be forwarded into the function at the call site
		template<typename t>
		constexpr auto as_passable()
		{
			if constexpr (std::template is_rvalue_reference_v<t> || std::template is_lvalue_reference_v<t>)
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
		struct store
		{
			typedef decltype(as_storable<t>()) type;
		};

		template<typename t>
		using store_t = typename store<t>::type::held;


		template<typename t>
		struct pass
		{
			typedef decltype(as_passable<t>()) type;
		};

		template<typename t>
		using pass_t = typename pass<t>::type::held;

		template<typename t>
		struct returned
		{
			typedef decltype(as_returnable<t>()) type;
		};

		template<typename t>
		using returned_t = typename returned<t>::type::held;


		//where t is the argument type in the function
		template<typename t>
		constexpr pass_t<t> storable_into_passable(store_t<t>&& x)
		{
			return pass_t<t>(std::move(x));
		}

		template<typename t>
		constexpr bool can_return()
		{
			return !std::is_same_v<void,returned_t<t>>;
		}

		template<typename t>
		constexpr returned_t<t> into_returnable(t&& x)
		{
			if constexpr (!can_return<t>())
			{
				return;
			}
			else if constexpr(std::is_lvalue_reference_v<t>)
			{
				return returned_t<t>(x);
			}
			else
			{
				return returned_t<t>(std::move(x));
			}
		}



		template<typename ret_t, typename...argts>
		std::function<returned_t<ret_t>(store_t<argts>...)> make_storable_call(std::function<ret_t(argts...)>&& f)
		{
			if constexpr(can_return<ret_t>())
			{
				return std::function<returned_t<ret_t>(store_t<argts>&&...)> {
					[f = std::move(f)](store_t<argts>...argvs)->returned_t<ret_t>
					{
						return into_returnable<ret_t>(f(storable_into_passable<argts>(std::move(argvs))...));
					}
				};
			}
			else
			{
				return std::function<void(store_t<argts>&&...)> {
					[f = std::move(f)](store_t<argts>&&...argvs) -> void
					{
						f(storable_into_passable<argts>(std::move(argvs))...);
					}
				};
			}
		}
	}
}
