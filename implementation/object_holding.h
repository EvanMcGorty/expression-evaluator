#pragma once

#include"type_asker.h"
#include"statement.h"
#include"type_name_mapping.h"

namespace expr
{
	namespace impl
	{

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

			virtual bool is_variable() const
			{
				return false;
			}

			virtual bool has_value() const
			{
				return false;
			}

			virtual std::string convert_into_string() = 0;

			virtual std::string string_view(name_set const& names) = 0;

			//returns true if this is an object and the held value was moved.
			virtual bool get(any_type_ask* tar) = 0;

			virtual bool can(std::type_info const& tar) const = 0;


			//returns true if this is an object and the held value was moved.
			virtual bool lazy_get(any_type_ask* tar) = 0;

			virtual bool lazy_can(std::type_info const& tar) const = 0;

			virtual ~any_elem_val()
			{

			}
		};



		//on a stack, this is what a literal pushes.
		class unparsed : public any_elem_val
		{
		public:

			unparsed(literal&& a)
			{
				val = std::move(a.data.val);
			}

			bool is_unparsed() const override
			{
				return true;
			}

			std::string convert_into_string() override
			{
				return std::string{ val };
			}

			std::string string_view(name_set const&) override
			{
				literal temp{ literal_value{std::string{val}} };
				return std::string("unparsed{") + temp.make_string() + "}";
			}

			bool lazy_get(any_type_ask* tar) override
			{
				tar->lazy_parse(val);
				return false;
			}

			bool get(any_type_ask* tar) override
			{
				tar->parse(val);
				return false;
			}

			bool lazy_can(std::type_info const&) const override
			{
				return true;
			}

			bool can(std::type_info const&) const override
			{
				return true;
			}

			std::string val;
		};

		class any_object;

		class value_elem_val : public any_elem_val
		{
		public:

			virtual bool has_value() const final override
			{
				return true;
			}

			virtual mu::virt<any_object> make_clone() = 0;

			virtual mu::virt<any_object> make_reference() = 0;

			virtual mu::virt<any_object> take_value() = 0;

			virtual mu::virt<any_object> unwrap() = 0;
		};


		//the object held by a variable
		//on a stack, this is what a function pushes.
		class any_object : public value_elem_val
		{
		public:
			virtual std::type_info const& get_type() const = 0;


			virtual mu::virt<any_object> as_non_trivially_destructible() && = 0;

			virtual bool can_trivially_destruct() const = 0;

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


		//indicates a successful function call for with return type void
		class void_object final : public any_object
		{
		public:
			std::type_info const& get_type() const override
			{
				return typeid(void);
			}

			std::string convert_into_string() override
			{
				return "";
			}
			
			std::string string_view(name_set const&) override
			{
				return "void_object{}";
			}

			mu::virt<any_object> as_non_trivially_destructible() && override
			{
				return mu::virt<any_object>::make<void_object>();
			}


			mu::virt<any_object> make_clone() override
			{
				return mu::virt<any_object>::make_nullval();
			}

			mu::virt<any_object> make_reference() override
			{
				return mu::virt<any_object>::make_nullval();
			}

			mu::virt<any_object> take_value() override
			{
				return mu::virt<any_object>::make_nullval();
			}
			
			mu::virt<any_object> unwrap() override
			{
				return mu::virt<any_object>::make_nullval();
			}

			bool can_trivially_destruct() const override
			{
				return true;
			}

			bool get(any_type_ask*) override
			{
				return false;
			}

			bool can(std::type_info const&) const override
			{
				return false;
			}


			bool lazy_get(any_type_ask*) override
			{
				return false;
			}

			bool lazy_can(std::type_info const&) const override
			{
				return false;
			}

		};


		


		template<typename t>
		class object_of final : public any_object
		{
		private:
			typedef typename type<t>::raw raw;

			raw& get_raw()
			{
				if constexpr(type<t>::is_ref())
				{
					return static_cast<raw&>(val);
				}
				else if constexpr(type<t>::is_val())
				{
					return val.wrapped;
				}
			}

		public:

			object_of(t&& a) :
				val(std::move(a))
			{
				static_assert(!std::is_const_v<t>,"object_of<t> should not have a const t");
				static_assert(!type<t>::is_raw());
			}

			std::string convert_into_string() override
			{
				return converter<raw>::print(get_raw());
			}

			std::string string_view(name_set const& names) override
			{
				return std::string("object_of{") + name_of<t>(names) + "(" + convert_into_string() + ")" + "}";
			}

			mu::virt<any_object> as_non_trivially_destructible() && override
			{
				if constexpr(std::is_trivially_destructible_v<raw>)
				{
					return mu::virt<any_object>::make<object_of<post_return_t<strong<raw>>>>(into_returnable(strong<raw>{std::move(get_raw())}));
				}
				else
				{
					return mu::virt<any_object>::make<object_of<t>>(std::move(val));
				}
			}

			bool can_trivially_destruct() const override
			{
				return std::is_trivially_destructible_v<t>;
			}

			mu::virt<any_object> make_clone() override
			{
				if constexpr(std::is_constructible_v<std::remove_const_t<raw>, raw&>)
				{
					return mu::virt<any_object>::make<object_of<val_wrap<std::remove_const_t<raw>>>>(into_returnable<std::remove_const_t<raw>>(raw(get_raw())));
				}
				else
				{
					return mu::virt<any_object>::make_nullval();
				}
			}

			mu::virt<any_object> make_reference() override
			{
				return mu::virt<any_object>::make<object_of<post_return_t<raw&>>>(into_returnable<raw&>(get_raw()));
			}

			virtual mu::virt<any_object> take_value() override
			{
				if constexpr(type<t>::is_ref() && std::is_const_v<raw>)
				{
					return mu::virt<any_object>::make_nullval();
				}
				else
				{
					return mu::virt<any_object>::make<object_of<post_return_t<raw>>>(into_returnable<raw>(std::move(get_raw())));
				}
			}

			mu::virt<any_object> unwrap() override
			{
				if constexpr(type_wrap_info<raw>::is())
				{
					if constexpr(!std::is_const_v<typename type_wrap_info<raw>::deref>)
					{
						if(type_wrap_info<raw>::has(get_raw()))
						{
							return mu::virt<any_object>::make<object_of<post_return_t<typename type_wrap_info<raw>::deref>>>(into_returnable(type_wrap_info<raw>::get(get_raw())));
						}
					}
				}
				return mu::virt<any_object>::make_nullval();
			}

			std::type_info const& get_type() const override
			{
				return typeid(t);
			}

			bool is_movable()
			{
				//if copying does the same as moving but leaves behind the old copy, then the origional doesn't need to be destroyed on the spot.
				//it must be trivially destructible though, otherwise the copy will be sent to the garbage and an extra destructor could end up running.
				if constexpr(std::is_trivially_copy_constructible_v<t> && std::is_trivially_move_constructible_v<t> && std::is_trivially_destructible_v<t>)
				{
					return false;
				}
				else
				{
					return true;
				}
			}

			bool lazy_get(any_type_ask* tar) override
			{
				if (tar->get_type() == typeid(t))
				{
					static_cast<type_ask_of<t>*>(tar)->gotten.emplace(std::move(val));
					return is_movable();
				}
				return false;
			}

			bool get(any_type_ask* tar) override
			{
				if constexpr(type<t>::is_val())
				{
					if (tar->get_type() == typeid(t))
					{
						static_cast<type_ask_of<t>*>(tar)->gotten.emplace(std::move(val));
						return is_movable();
					}
				}
				if (tar->get_type() == typeid(ref_wrap<raw>))
				{
					static_cast<type_ask_of<ref_wrap<raw>>*>(tar)->gotten.emplace(into_returnable<raw&>(get_raw()));
					return false;
				}
				if constexpr(!std::is_const_v<raw>)
				{
					if (tar->get_type() == typeid(ref_wrap<raw const>))
					{
						static_cast<type_ask_of<ref_wrap<raw const>>*>(tar)->gotten.emplace(into_returnable<raw const&>(get_raw()));
						return false;
					}
				}
				return false;
			}

			bool lazy_can(std::type_info const& tar) const override
			{
				if (tar == typeid(t))
				{
					return true;
				}
				return false;
			}

			bool can(std::type_info const& tar) const override
			{
				if (tar == typeid(t))
				{
					return true;
				}
				if (tar == typeid(ref_wrap<raw>))
				{
					return true;
				}
				if constexpr(!std::is_const_v<raw>)
				{
					if (tar == typeid(ref_wrap<raw const>))
					{
						return true;
					}
				}
				return false;
			}


			t val;
		};


		using object_holder = mu::virt<any_object>;

		template<typename t>
		mu::virt<object_of<post_return_t<t>>> make_object(t&& a)
		{
			return mu::virt<object_of<post_return_t<t>>>::template make<object_of<post_return_t<t>>>(into_returnable<t>(std::forward<t>(a)));
		}



		struct predeclared_object_result
		{
			predeclared_object_result(object_holder&& a) :
				wrapped(std::move(a))
			{}
			object_holder wrapped;
		};


		template<typename t>
		predeclared_object_result type_operations_for<t>::make_from_string(std::string::const_iterator& start, std::string::const_iterator stop)
		{
			std::optional<t> g{ converter<t>::parse(start,stop) };
			if (g)
			{
				return predeclared_object_result{ make_object(g) };
			}
			else
			{
				return predeclared_object_result{ object_holder::make_nullval() };
			}
		}

		/*template<typename t>
		constexpr type_operations_for<t> make_type_operations()
		{
			return type_operations_for<t>{};
		}*/


		//on a stack, this is what a variable pushes (unless if the variable is also being popped).
		class variable_reference : public value_elem_val
		{
		public:


			variable_reference(object_holder* a)
			{
				ref = a;
			}

			bool is_variable() const override
			{
				return true;
			}


			mu::virt<any_object> make_clone() override
			{
				if (!ref->is_nullval())
				{
					return (**ref).make_clone();
				}
				else
				{
					return mu::virt<any_object>::make_nullval();
				}
			}

			mu::virt<any_object> make_reference() override
			{
				if (!ref->is_nullval())
				{
					return (**ref).make_reference();
				}
				else
				{
					return mu::virt<any_object>::make_nullval();
				}
			}

			mu::virt<any_object> take_value() override
			{
				if (!ref->is_nullval())
				{
					return (**ref).take_value();
				}
				else
				{
					return mu::virt<any_object>::make_nullval();
				}
			}

			mu::virt<any_object> unwrap() override
			{
				if (!ref->is_nullval())
				{
					return (**ref).unwrap();
				}
				else
				{
					return mu::virt<any_object>::make_nullval();
				}
			}



			bool get(any_type_ask* tar) override
			{
				if (!ref->is_nullval())
				{
					if ((**ref).get(tar))
					{
						*ref = object_holder::make_nullval();
					}
				}

				return false;
			}


			bool can(std::type_info const& tar) const override
			{
				if (ref->is_nullval())
				{
					return false;
				}
				else
				{
					return (**ref).can(tar);
				}
			}

			bool lazy_get(any_type_ask* tar) override
			{
				if (!ref->is_nullval())
				{
					if ((**ref).lazy_get(tar))
					{
						*ref = object_holder::make_nullval();
					}
				}

				return false;
			}


			bool lazy_can(std::type_info const& tar) const override
			{
				if (ref->is_nullval())
				{
					return false;
				}
				else
				{
					return (**ref).lazy_can(tar);
				}
			}

			std::string convert_into_string() override
			{
				if (ref->is_nullval())
				{
					return "";
				}
				else
				{
					return (**ref).convert_into_string();
				}
			}

			std::string string_view(name_set const& names) override
			{
				if (ref->is_nullval())
				{
					return "empty_variable";
				}
				else
				{
					return "variable_with_" + (**ref).string_view(names);
				}
			}

			object_holder* ref;
		};

		using any_value = mu::virt<any_elem_val>;



	}
}
