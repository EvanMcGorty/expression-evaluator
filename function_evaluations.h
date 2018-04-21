
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


		value_holder get_value(stack_elem& a)
		{
			assert(a.is_nullval() || a->has_value());
			if (a.is_nullval())
			{
				return value_holder::make_nullval();
			}
			else if (a->is_reference())
			{
				return std::move(*a.downcast_get<value_reference>()->ref);
			}
			else if (a->is_object())
			{
				return std::move(std::move(a).downcast<any_object>());
			}
			else
			{
				assert(false);
				return value_holder::make_nullval();
			}
		}

		void set_value(stack_elem& a, value_holder&& b)
		{
			assert(a.is_nullval() || a->has_value());
			if (a.is_nullval() || a->is_object())
			{
				a = std::move(b);
			}
			else if (a->is_reference())
			{
				(*a.downcast_get<value_reference>()->ref) = std::move(b);
			}
			else
			{
				assert(false);
			}
		}

		class variable_value_stack;

		class stack
		{
			template<typename tup_t, size_t ind, typename t, typename...ts>
			void set_rest(tup_t& a)
			{
				if (stuff[stuff.size() + ind - std::tuple_size<tup_t>::value].is_nullval())
				{
					std::get<ind>(a) = std::nullopt;
				}
				else
				{
					stack_elem& cur = stuff[stuff.size() + ind - std::tuple_size<tup_t>::value];
					type_ask_of<t> ask;
					if (cur->get(&ask))
					{
						cur = stack_elem::make_nullval();
						std::get<ind>(a) = std::move(ask.gotten);
					}
					else if constexpr (std::is_pointer_v<t>)
					{
						if (ask.pointed_to_value)
						{
							cur = stack_elem::make<object_of<std::remove_const_t<typename std::remove_pointer_t<t>>>>(std::move(ask.pointed_to_value->val));
							std::get<ind>(a) = &cur.downcast_get<object_of<std::remove_const_t<typename std::remove_pointer_t<t>>>>()->val;
						}
						else
						{
							std::get<ind>(a) = std::move(ask.gotten);
						}
					}
					else
					{
						std::get<ind>(a) = std::move(ask.gotten);
					}
				}
				if constexpr(sizeof...(ts) > 0)
				{
					set_rest<tup_t, ind + 1, ts...>(a);
				}
			}



		public:
			std::vector<stack_elem> stuff;

			template<typename tup_t, typename...ts>
			void set_from_front(tup_t& a)
			{
				static_assert(std::is_same_v<tup_t, std::tuple<std::optional<ts>...>>);
				set_rest<tup_t, 0, ts...>(a); //easier not to have this all in one function
			}

			template<size_t ind, typename t, typename...ts>
			bool check_from_front() const
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
						return ret && check_from_front<ind+1,ts...>();
					}
				}
			}

			void clear_front(size_t a, variable_value_stack& garbage);


		};


		class any_callable
		{
		public:
			virtual value_holder try_perform(stack& a, size_t args_to_take) = 0;
			virtual void put_type(std::ostream& target) const = 0;
			virtual mu::virt<any_callable> add_layer(mu::virt<any_callable>&& layer) && = 0;
			virtual ~any_callable()
			{}
		};

		using held_callable = mu::virt<any_callable>;

		template<typename t,typename...ts>
		void put_types(std::ostream& target)
		{
			if constexpr(sizeof...(ts) > 0)
			{
				target << demangle(typeid(t).name()) << ", ";
				put_types<ts...>(target);
			}
			else
			{
				target << demangle(typeid(t).name());
			}
		}

		struct dummy_argument
		{};

		template<typename ret_t, typename...args>
		class callable_of : public any_callable
		{
			using use_tuple_type = std::tuple<store_t<args>...>;
			using arg_tuple_type = std::tuple<std::optional<store_t<args>>...>;
		public:

			callable_of(std::function<ret_t(args...)>&& f)
			{
				target = make_storable_call(std::move(f));
			}

			callable_of(dummy_argument,std::function<return_t<ret_t>(store_t<args>&&...)>&& f)
			{
				target = std::move(f);
			}

			held_callable add_layer(held_callable&& layer) && override;

			void put_type(std::ostream& target) const override
			{
				target << '(';
				if constexpr(sizeof...(args) > 0)
				{
					put_types<store_t<args>...>(target);
				}
				target << ") -> " << demangle(typeid(return_t<ret_t>).name());
			}

			//when the stack is not popped from, it is the callers responsibility to manage garbage variables
			value_holder try_perform(stack& a, size_t args_to_take) override
			{
				if (sizeof...(args) != args_to_take)
				{
					return value_holder::make_nullval();
				}
				assert(a.stuff.size() >= sizeof...(args));

				if constexpr(sizeof...(args) > 0)
				{
					arg_tuple_type to_use;
					a.set_from_front<arg_tuple_type, store_t<args>...>(to_use);
					std::optional<use_tuple_type> might_use = can_perform<0, args...>(std::move(to_use));

					if (might_use)
					{
						if constexpr(!can_return<ret_t>())
						{
							do_call(std::move(*might_use));
							return value_holder::make_nullval();
						}
						else
						{
							return value_holder::make<object_of<return_t<ret_t>>>(do_call(std::move(*might_use)));
						}
					}
					else
					{
						return value_holder::make_nullval();
					}
				}
				else
				{
					if constexpr (!can_return<ret_t>())
					{
						target();
						return value_holder::make_nullval();
					}
					else
					{
						return value_holder::make<object_of<return_t<ret_t>>>(do_call(use_tuple_type{}));
					}
				}
			}

			~callable_of()
			{
			}


			std::function<return_t<ret_t>(store_t<args>&&...)> target;

		private:

			template<size_t ind = 0, typename t, typename...ts>
			static std::optional<std::tuple<store_t<t>, store_t<ts>...>> can_perform(arg_tuple_type&& tar)
			{
				if constexpr(sizeof...(ts) == 0)
				{
					if (std::get<ind>(tar))
					{
						return std::optional<std::tuple<store_t<t>, store_t<ts>...>>(std::move(std::tuple<store_t<t>>{std::move(*std::get<ind>(tar))}));
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
						auto rest = can_perform<ind + 1, ts...>(std::move(tar));
						if (rest)
						{
							return std::tuple_cat(std::tuple<store_t<t>>(std::move(*std::get<ind>(tar))), std::move(*rest));
						}
					}
					return std::nullopt;
				}
			}

			return_t<ret_t> do_call(use_tuple_type&& a)
			{
				return call(std::function<return_t<ret_t>(store_t<args>&&...)>(target), std::move(a));
			}
		};


		class manual_callable : public any_callable
		{
		public:
			manual_callable(std::function<value_holder(std::vector<stack_elem>&)>&& a)
			{
				target = a;
			}

			void put_type(std::ostream& target) const override
			{
				target << "(...)->?";
			}

			held_callable add_layer(held_callable&& layer) &&
			{
				//because try_perform never fails, the next layer would never need to be called
				return held_callable::make<manual_callable>(std::move(target));
			}

			value_holder try_perform(stack& a, size_t args_to_take) override
			{
				assert(a.stuff.size() >= args_to_take);
				std::vector<stack_elem> to_call;
				to_call.reserve(args_to_take);
				for (int i = a.stuff.size() - args_to_take; i != a.stuff.size(); ++i)
				{
					to_call.emplace_back(std::move(a.stuff[i]));
				}
				auto ret = target(to_call);
				for (int i = 0; i != to_call.size(); ++i)
				{
					a.stuff[a.stuff.size() - args_to_take + i] = std::move(to_call[i]);
				}
				return ret;
			}

		private:
			std::function<value_holder(std::vector<stack_elem>&)> target;
		};


		template<typename ret_t, typename...args>
		class multi_callable_of : public callable_of<ret_t, args...>
		{
		public:
			multi_callable_of(std::function<ret_t(args...)>&& f, held_callable&& old) :
				next(std::move(old)),
				callable_of<ret_t, args...>(std::move(f))
			{}

			multi_callable_of(dummy_argument,std::function<return_t<ret_t>(store_t<args>&&...)>&& f, held_callable&& old) :
				next(std::move(old)),
				callable_of<ret_t, args...>(dummy_argument{},std::move(f))
			{}

			held_callable add_layer(held_callable&& layer) && override
			{
				return std::move(*next).add_layer(std::move(layer));
			}

			value_holder try_perform(stack& a, size_t args_to_take) override
			{
				if (sizeof...(args) != args_to_take)
				{
					return next->try_perform(a, args_to_take);
				}
				assert(a.stuff.size() >= sizeof...(args));

				if constexpr(sizeof...(args) == 0)
				{
					return callable_of<ret_t, args...>::try_perform(a, args_to_take);
				}
				else if (a.check_from_front<0, args...>())
				{
					return callable_of<ret_t, args...>::try_perform(a, args_to_take);
				}
				else
				{
					return next->try_perform(a, args_to_take);
				}
			}


			void put_type(std::ostream& target) const override
			{
				target << "\\ ";
				callable_of<ret_t, args...>::put_type(target);
				target << " | ";
				next->put_type(target);
			}

		private:
			held_callable next;
		};

		template<typename ret_t,typename...args>
		inline held_callable callable_of<ret_t,args...>::add_layer(held_callable&& layer) &&
		{
			return held_callable::make<multi_callable_of<ret_t, args...>>(dummy_argument{},std::move(target), std::move(layer));
		}

	}
}