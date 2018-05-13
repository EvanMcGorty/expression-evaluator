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

			virtual bool can(std::type_info const& tar) const = 0;

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
				value = std::move(a.data.value);
			}

			bool is_unparsed() const override
			{
				return true;
			}

			std::string convert_into_string() const override
			{
				return std::string{ value };
			}

			std::string string_view(name_set const& names) const override
			{
				literal temp{ literal_value{std::string{value}} };
				return std::string("unparsed{") + temp.make_string() + "}";
			}

			bool get(any_type_ask* tar) override
			{
				tar->parse(value);
				return false;
			}

			bool can(std::type_info const& tar) const override
			{
				return true;
			}

			std::string value;
		};


		class value_elem_val : public any_elem_val
		{
			virtual bool has_value() const final override
			{
				return true;
			}
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


		template <typename t>
		struct pointer
		{
			static constexpr bool is()
			{
				return false;
			}
		};

		template <typename t>
		struct pointer<t*>
		{
			static constexpr bool is()
			{
				return true;
			}

			typedef t deref;
		};

		template <typename t>
		struct pointer<std::unique_ptr<t>>
		{
			static constexpr bool is()
			{
				return true;
			}

			typedef t deref;
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
					return mu::virt<any_object>::make<object_of<std::unique_ptr<t>>>(std::make_unique<t>(std::move(val)));
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

			std::type_info const& get_type() const override
			{
				return type();
			}

			bool get(any_type_ask* tar) override
			{
				if (tar->get_type() == typeid(t*))
				{
					static_cast<type_ask_of<t*>*>(tar)->gotten.emplace(&val);
					return false;
				}
				else if (tar->get_type() == typeid(t const*))
				{
					static_cast<type_ask_of<t const*>*>(tar)->gotten.emplace(&val);
					return false;
				}
				else if (tar->get_type() == typeid(t))
				{
					//if copying does the same as moving but leaves behind the old copy, then it should be done instead
					//it must be trivially destructible though, otherwise the copy will be sent to the garbage and an extra destructor could end up running.
					if constexpr(std::is_trivially_copy_constructible_v<t> && std::is_trivially_move_constructible_v<t> && std::is_trivially_destructible_v<t>)
					{
						static_cast<type_ask_of<t>*>(tar)->gotten.emplace(val);
						return false;
					}
					else
					{
						static_cast<type_ask_of<t>*>(tar)->gotten.emplace(std::move(val));
						return true;
					}
				}
				else if constexpr (pointer<t>::is()) //to return a t*/t& as a t&& or t const&
				{
					if (tar->get_type() == typeid(typename pointer<t>::deref const*))
					{
						static_cast<type_ask_of<typename pointer<t>::deref const*>*>(tar)->gotten.emplace(std::move(&*val));
					}
					else if (tar->get_type() == typeid(typename pointer<t>::deref*))
					{
						static_cast<type_ask_of<typename pointer<t>::deref*>*>(tar)->gotten.emplace(std::move(&*val));
					}
					else if (tar->get_type() == typeid(typename pointer<t>::deref))
					{
						static_cast<type_ask_of<typename pointer<t>::deref>*>(tar)->gotten.emplace(std::move(*val));
					}
					return false;
				}
				else
				{
					return false;
				}
			}

			bool can(std::type_info const& tar) const override
			{
				if (tar == typeid(t*))
				{
					return true;
				}
				else if (tar == typeid(t const*))
				{
					return true;
				}
				else if (tar == typeid(t))
				{
					return true;
				}
				else if constexpr(pointer<t>::is())
				{
					if (tar == typeid(typename pointer<t>::deref const*))
					{
						return true;
					}
					else if (tar == typeid(typename pointer<t>::deref*))
					{
						return true;
					}
					else if (tar == typeid(typename pointer<t>::deref))
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
					return false;
				}
			}


			t val;
		};


		using value_holder = mu::virt<any_object>;


		//on a stack, this is what a variable pushes (unless if the variable is also being popped).
		class value_reference : public value_elem_val
		{
		public:

			value_reference(value_holder* a)
			{
				ref = a;
			}

			bool is_reference() const override
			{
				return true;
			}

			bool get(any_type_ask* tar) override
			{
				if (!ref->is_nullval())
				{
					if ((**ref).get(tar))
					{
						*ref = value_holder::make_nullval();
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
					return "empty_variable";
				}
				else
				{
					return "variable_with_" + (**ref).string_view(names);
				}
			}

			value_holder* ref;
		};



	}
}