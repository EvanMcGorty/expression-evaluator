#pragma once

#include"type_asker.h"

#include"statement.h"

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

			//returns true if this is an object and the held value was moved.
			virtual bool get(any_type_ask* tar) = 0;

			virtual ~any_elem_val()
			{

			}
		};

		class value_elem_val : public any_elem_val
		{
			virtual bool has_value() const final override
			{
				return true;
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

			bool get(any_type_ask* tar) override
			{
				tar->parse(value);
				return false;
			}

			std::string value;
		};


		//the object held by a variable
		//on a stack, this is what a function pushes.
		class any_object : public value_elem_val
		{
		public:
			virtual std::type_info const& get_type() const = 0;


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
				else if constexpr (std::is_pointer_v<t>) //to return a t*/t& as a t&& or t const&
				{
					if (tar->get_type() == typeid(std::remove_pointer_t<t> const*))
					{
						static_cast<type_ask_of<std::remove_pointer_t<t> const*>*>(tar)->gotten.emplace(std::move(val));
						//normally here  this should return true, but a pointer is always still valid/untouched after it is std::moved, it is trivially copy/move constructible and trivially destructible
					}
					if (tar->get_type() == typeid(std::remove_pointer_t<t>))
					{
						static_cast<type_ask_of<std::remove_pointer_t<t>>*>(tar)->gotten.emplace(std::move(*val));
					}
					return false;
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


			value_holder* ref;
		};



	}
}