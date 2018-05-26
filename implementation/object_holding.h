#pragma once

#include"type_asker.h"
#include"statement.h"
#include"type_demangle.h"

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

			virtual bool is_reference() const
			{
				return false;
			}

			virtual bool has_value() const
			{
				return false;
			}

			virtual std::string convert_into_string() const = 0;

			virtual std::string string_view(name_set const& names) const = 0;

			//returns true if this is an object and the held value was moved.
			virtual bool get(any_type_ask* tar) = 0;

			virtual bool smart_get(any_type_ask* tar)
			{
				return get(tar);
			}

			virtual bool can(std::type_info const& tar) const = 0;

			virtual bool smart_can(std::type_info const& tar) const
			{
				return can(tar);
			}

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

			std::string convert_into_string() const override
			{
				return std::string{ val };
			}

			std::string string_view(name_set const& names) const override
			{
				literal temp{ literal_value{std::string{val}} };
				return std::string("unparsed{") + temp.make_string() + "}";
			}

			bool get(any_type_ask* tar) override
			{
				tar->parse(val);
				return false;
			}

			bool can(std::type_info const& tar) const override
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

			virtual mu::virt<any_object> make_clone() const = 0;

			virtual mu::virt<any_object> take_referenced() const = 0;
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

			std::string convert_into_string() const override
			{
				return "";
			}
			
			std::string string_view(name_set const& names) const override
			{
				return "void_object{}";
			}

			mu::virt<any_object> as_non_trivially_destructible() && override
			{
				return mu::virt<any_object>::make<void_object>();
			}

			mu::virt<any_object> make_clone() const override
			{
				return mu::virt<any_object>::make_nullval();
			}

			mu::virt<any_object> take_referenced() const override
			{
				return mu::virt<any_object>::make_nullval();
			}

			bool can_trivially_destruct() const override
			{
				return true;
			}

			bool get(any_type_ask* tar) override
			{
				return false;
			}

			bool can(std::type_info const& tar) const override
			{
				return false;
			}

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

			object_of(t&& a) :
				val(std::move(a))
			{
				static_assert(!std::is_const_v<t>,"object_of<t> should not have a const t");
			}

			std::string convert_into_string() const override
			{
				return converter<t>::print(val);
			}

			std::string string_view(name_set const& names) const override
			{
				return std::string("object_of{") + name_of<t>(names) + "(" + converter<t>::print(val) + ")" + "}";
			}

			mu::virt<any_object> as_non_trivially_destructible() && override
			{
				if constexpr(std::is_trivially_destructible_v<t>)
				{
					return mu::virt<any_object>::make<object_of<strong<t>>>(strong<t>(std::move(val)));
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

			mu::virt<any_object> make_clone() const override
			{
				if constexpr(std::is_copy_constructible<t>::value)
				{
					return mu::virt<any_object>::make<object_of<t>>(t{ val });
				}
				else
				{
					return mu::virt<any_object>::make_nullval();
				}
			}

			mu::virt<any_object> take_referenced() const override
			{
				if constexpr(type_wrap_info<t>::is())
				{
					if constexpr(!std::is_const_v<typename type_wrap_info<t>::deref>)
					{
						if(type_wrap_info<t>::get(val) != nullptr)
						{
							return mu::virt<any_object>::make<object_of<typename type_wrap_info<t>::deref>>(std::move(*type_wrap_info<t>::get(val)));
						}
					}
				}
				return mu::virt<any_object>::make_nullval();
			}

			std::type_info const& get_type() const override
			{
				return type();
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

			bool get(any_type_ask* tar) override
			{
				if (tar->get_type() == typeid(t))
				{
					static_cast<type_ask_of<t>*>(tar)->gotten.emplace(std::move(val));
					return is_movable();
				}
				else if constexpr (type_wrap_info<t>::is()) //for implicit conversions
				{
					if (tar->get_type() == typeid(typename type_wrap_info<t>::deref*))
					{
						static_cast<type_ask_of<typename type_wrap_info<t>::deref*>*>(tar)->gotten.emplace(std::move(type_wrap_info<t>::get(val)));
						return false;
					}
					if constexpr(!std::is_const_v<typename type_wrap_info<t>::deref>)
					{
						if (tar->get_type() == typeid(typename type_wrap_info<t>::deref const*))
						{
							static_cast<type_ask_of<typename type_wrap_info<t>::deref const*>*>(tar)->gotten.emplace(std::move(type_wrap_info<t>::get(val)));
							return false;
						}
					}
					return false;
				}
				return false;
			}

			bool smart_get(any_type_ask* tar) override
			{
				if (tar->get_type() == typeid(t*))
				{
					static_cast<type_ask_of<t*>*>(tar)->gotten.emplace(&val);
					return false;
				}
				if constexpr(!std::is_const_v<t>)
				{
					if (tar->get_type() == typeid(t const*))
					{
						static_cast<type_ask_of<t const*>*>(tar)->gotten.emplace(&val);
						return false;
					}
				}
				return get(tar);
			}
			

			bool can(std::type_info const& tar) const override
			{
				if (tar == typeid(t))
				{
					return true;
				}
				if constexpr(type_wrap_info<t>::is())
				{
					if (tar == typeid(typename type_wrap_info<t>::deref*))
					{
						return true;
					}
					if constexpr(!std::is_const_v<typename type_wrap_info<t>::deref>)
					{
						if (tar == typeid(typename type_wrap_info<t>::deref const*))
						{
							return true;
						}
					}
					return false;
				}
				return false;
			}

			bool smart_can(std::type_info const& tar) const override
			{
				if (tar == typeid(t*))
				{
					return true;
				}
				if constexpr(!std::is_const_v<t>)
				{
					if (tar == typeid(t const*))
					{
						return true;
					}
				}
				return can(tar);
			}


			t val;
		};


		using object_holder = mu::virt<any_object>;


		//on a stack, this is what a variable pushes (unless if the variable is also being popped).
		class value_reference : public value_elem_val
		{
		public:


			value_reference(object_holder* a)
			{
				ref = a;
			}

			bool is_reference() const override
			{
				return true;
			}


			object_holder make_clone() const override
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

			object_holder take_referenced() const override
			{
				if (!ref->is_nullval())
				{
					return (**ref).take_referenced();
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
					if ((**ref).smart_get(tar))
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
					return (**ref).smart_can(tar);
				}
			}

			std::string convert_into_string() const override
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

			std::string string_view(name_set const& names) const override
			{
				if (ref->is_nullval())
				{
					return "reference_to_empty_object";
				}
				else
				{
					return "reference_to_" + (**ref).string_view(names);
				}
			}

			object_holder* ref;
		};

		using any_value = mu::virt<any_elem_val>;



	}
}