
#pragma once
#include<unordered_map>

#include"tuple_call_unpack.h"
#include"call_typing.h"
#include"object_holding.h"

namespace expr
{
	namespace impl
	{

		using stack_elem = mu::virt<any_elem_val>;


		object_holder get_value(stack_elem& a)
		{
			assert_with_generic_logic_error(a.is_nullval() || a->has_value());
			if (a.is_nullval())
			{
				return object_holder::make_nullval();
			}
			else if (a->is_variable())
			{
				return std::move(*a.downcast_get<variable_reference>()->ref);
			}
			else if (a->is_object())
			{
				return std::move(a).downcast<any_object>();
			}
			else
			{
				assert_with_generic_logic_error(false);
				return object_holder::make_nullval();
			}
		}

		void set_value(stack_elem& a, object_holder&& b)
		{
			assert_with_generic_logic_error(a.is_nullval() || a->has_value());
			if (a.is_nullval() || a->is_object())
			{
				a = std::move(b);
			}
			else if (a->is_variable())
			{
				(*a.downcast_get<variable_reference>()->ref) = std::move(b);
			}
			else
			{
				assert_with_generic_logic_error(false);
			}
		}

		class variable_value_stack;

		template<typename t>
		std::optional<t> smart_take_elem(stack_elem& cur)
		{
			if (cur.is_nullval())
			{
				return std::nullopt;
			}
			else
			{
				type_ask_of<t> ask;
				bool has_moved = cur->get(&ask);
				if (has_moved)
				{
					cur = stack_elem::make_nullval();
					if (ask.gotten)
					{
						return std::optional<t>{std::move(*ask.gotten)};
					}
					else
					{
						return std::nullopt;
					}
				}
				else if constexpr (type<t>::is_ref())
				{
					if (ask.pointed_to_value)
					{
						auto&& new_cur = make_object<std::remove_const_t<typename type<t>::raw>>(std::move(ask.pointed_to_value->val));
						std::optional<t> ret{ t(new_cur->val.wrapped) };
						cur = std::move(new_cur);
						return ret;
					}
					else
					{
						if (ask.gotten)
						{
							return std::optional<t>(std::move(*ask.gotten));
						}
						else
						{
							return std::nullopt;
						}
					}
				}
				else
				{
					if (ask.gotten)
					{
						return std::optional<t>(std::move(*ask.gotten));
					}
					else
					{
						return std::nullopt;
					}
				}
			}
		}

		class stack
		{
			template<typename tup_t, size_t ind, typename t, typename...ts>
			void smart_set_rest(tup_t& a)
			{
				std::optional<t>&& temp = smart_take_elem<t>(stuff[stuff.size() + ind - std::tuple_size_v<tup_t>]);
				if(temp)
				{
					std::get<ind>(a).emplace(std::move(*temp));
				}
				
				if constexpr(sizeof...(ts) > 0)
				{
					smart_set_rest<tup_t, ind + 1, ts...>(a);
				}
			}



		public:
			std::vector<stack_elem> stuff;

			template<typename tup_t, typename...ts>
			void smart_set_from_front(tup_t& a)
			{
				static_assert(std::is_same_v<tup_t, std::tuple<std::optional<ts>...>>);
				smart_set_rest<tup_t, 0, ts...>(a); //easier not to have this all in one function
			}

			template<size_t ind, typename t, typename...ts>
			bool smart_check_from_front() const
			{
				if (stuff[stuff.size() - 1 - sizeof...(ts)].is_nullval())
				{
					return false;
				}
				else
				{
					stack_elem const& cur = stuff[stuff.size() - 1 - sizeof...(ts)];
					bool ret = cur->can(typeid(t));
					if constexpr(sizeof...(ts) == 0)
					{
						return ret;
					}
					else
					{
						return ret && smart_check_from_front<ind+1,ts...>();
					}
				}
			}


		};


		class any_callable
		{
		public:
			any_callable() = default;
			any_callable(any_callable const&) = default;
			any_callable(any_callable&&) = default;

			virtual object_holder try_perform(stack& a, size_t args_to_take) = 0;
			virtual bool can_perform(stack const& a, size_t args_to_take) const = 0;
			virtual void put_type(std::ostream& target, type_info_set const& from) const = 0;
			virtual mu::virt<any_callable> add_layer(mu::virt<any_callable>&& tail) && = 0;
			virtual ~any_callable()
			{}
		};

		using held_callable = mu::virt<any_callable>;

		template<class base_callable>
		class layered : public base_callable
		{
		public:

			layered(base_callable&& f, held_callable&& tail) :
				base_callable(std::move(f)),
				next(std::move(tail))
			{
				static_assert(std::is_base_of_v<any_callable, base_callable>);
			}

			held_callable add_layer(held_callable&& tail) && override
			{
				auto new_next = std::move(*next).add_layer(std::move(tail));
				return held_callable::make<layered<base_callable>>(std::move(*this), std::move(new_next));
			}

			object_holder try_perform(stack& a, size_t args_to_take) override
			{
				if (base_callable::can_perform(a, args_to_take))
				{
					return base_callable::try_perform(a, args_to_take);
				}
				else
				{
					return next->try_perform(a, args_to_take);
				}
			}

			bool can_perform(stack const& a, size_t args_to_take) const override
			{
				return base_callable::can_perform(a, args_to_take) || next->can_perform(a, args_to_take);
			}


			void put_type(std::ostream& into, type_info_set const& from) const override
			{
				into << "\\ ";
				base_callable::put_type(into, from);
				into << " | ";
				next->put_type(into, from);
			}

		private:
			held_callable next;
		};


		template<typename t,typename...ts>
		void put_types(std::ostream& target, type_info_set const& from)
		{
			if constexpr(sizeof...(ts) > 0)
			{
				target << name_of<t>(from) << ", ";
				put_types<ts...>(target,from);
			}
			else
			{
				target << name_of<t>(from);
			}
		}


		template<typename ret_t, typename...args>
		class smart_callable : public any_callable
		{
			using use_tuple_type = std::tuple<pre_call_t<args>...>;
			using arg_tuple_type = std::tuple<std::optional<pre_call_t<args>>...>;
		public:

			smart_callable() = default;
			smart_callable(smart_callable&&) = default;
			smart_callable(smart_callable const&) = default;


			smart_callable(std::function<post_return_t<ret_t>(pre_call_t<args>&&...)>&& f)
			{
				target = std::move(f);
			}

			held_callable add_layer(held_callable&& tail) && override
			{
				return held_callable::make<layered<smart_callable<ret_t, args...>>>(smart_callable<ret_t, args...>{std::move(target)}, std::move(tail));
			}

			void put_type(std::ostream& into, type_info_set const& from) const override
			{
				into << '(';
				if constexpr(sizeof...(args) > 0)
				{
					put_types<pre_call_t<args>...>(into,from);
				}

				if constexpr(can_return<ret_t>())
				{
					into << ") -> " << name_of<post_return_t<ret_t>>(from);
				}
				else
				{
					into << ")";
				}
			}

			//when the stack is not popped from, it is the callers responsibility to manage garbage variables
			object_holder try_perform(stack& a, size_t args_to_take) override
			{
				if (sizeof...(args) != args_to_take)
				{
					return object_holder::make_nullval();
				}
				assert_with_generic_logic_error(a.stuff.size() >= sizeof...(args));

				if constexpr(sizeof...(args) > 0)
				{
					arg_tuple_type to_use;
					a.smart_set_from_front<arg_tuple_type, pre_call_t<args>...>(to_use);
					std::optional<use_tuple_type> might_use = prepare_arguments<0, args...>(std::move(to_use));

					if (might_use)
					{
						if constexpr(!can_return<ret_t>())
						{
							do_call(std::move(*might_use));
							return object_holder::make<void_object>(); //indicates a successful function call even though the return type is void
						}
						else
						{
							return object_holder::make<object_of<post_return_t<ret_t>>>(do_call(std::move(*might_use)));
						}
					}
					else
					{
						return object_holder::make_nullval();
					}
				}
				else
				{
					if constexpr (!can_return<ret_t>())
					{
						target();
						return object_holder::make<void_object>(); //indicates a successful function call even though the return type is void
					}
					else
					{
						return object_holder::make<object_of<post_return_t<ret_t>>>(do_call(use_tuple_type{}));
					}
				}
			}

			bool can_perform(stack const& a, size_t args_to_take) const override
			{
				if (sizeof...(args) != args_to_take)
				{
					return false;
				}

				assert_with_generic_logic_error(a.stuff.size() >= sizeof...(args));
				
				if constexpr(!(sizeof...(args) == 0))
				{
					if (a.smart_check_from_front<0, pre_call_t<args>...>())
					{
						return true;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return true;
				}

			}

			~smart_callable() override
			{
			}


			std::function<post_return_t<ret_t>(pre_call_t<args>&&...)> target;

		private:

			template<size_t ind = 0, typename t, typename...ts>
			static std::optional<std::tuple<pre_call_t<t>, pre_call_t<ts>...>> prepare_arguments(arg_tuple_type&& tar)
			{
				if constexpr(sizeof...(ts) == 0)
				{
					if (std::get<ind>(tar))
					{
						return std::optional<std::tuple<pre_call_t<t>, pre_call_t<ts>...>>(std::move(std::tuple<pre_call_t<t>>{std::move(*std::get<ind>(tar))}));
					}
					else
					{
						return std::nullopt;
					}
				}
				else
				{
					if (std::get<ind>(tar))
					{
						auto rest = prepare_arguments<ind + 1, ts...>(std::move(tar));
						if (rest)
						{
							return std::tuple_cat(std::tuple<pre_call_t<t>>(std::move(*std::get<ind>(tar))), std::move(*rest));
						}
					}
					return std::nullopt;
				}
			}

			post_return_t<ret_t> do_call(use_tuple_type&& a)
			{
				return call(std::function<post_return_t<ret_t>(pre_call_t<args>&&...)>(target), std::move(a));
			}
		};


		class manual_callable : public any_callable
		{
		public:
			manual_callable(std::function<object_holder(std::vector<stack_elem>&)>&& a)
			{
				target = a;
			}

			void put_type(std::ostream& into, type_info_set const&) const override
			{
				into << "(...)->?";
			}

			held_callable add_layer(held_callable&&) && override
			{
				//because try_perform never fails, the next layer would never need to be called
				return held_callable::make<manual_callable>(std::move(target));
			}

			object_holder try_perform(stack& a, size_t args_to_take) override
			{
				assert_with_generic_logic_error(a.stuff.size() >= args_to_take);
				std::vector<stack_elem> to_call;
				to_call.reserve(args_to_take);
				for (size_t i = a.stuff.size() - args_to_take; i != a.stuff.size(); ++i)
				{
					to_call.emplace_back(std::move(a.stuff[i]));
				}
				auto ret = target(to_call);
				for (size_t i = 0; i != to_call.size(); ++i)
				{
					a.stuff[a.stuff.size() - args_to_take + i] = std::move(to_call[i]);
				}
				return ret;
			}

			bool can_perform(stack const&, size_t) const override
			{
				return true;
			}

		private:
			std::function<object_holder(std::vector<stack_elem>&)> target;
		};


	}
}
