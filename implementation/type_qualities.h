#pragma once
#include<memory>

namespace expr
{
	namespace impl
	{

		template <typename t>
		struct type_wrap_info
		{
			static constexpr bool is()
			{
				return false;
			}
		};

		template <typename t>
		struct type_wrap_info<t*>
		{
			static constexpr bool is()
			{
				return true;
			}

			static std::string suffix()
			{
				return "-ref";
			}

			typedef t deref;

			static deref* get(t*& a)
			{
				return a;
			}

		};

		template <typename t>
		struct type_wrap_info<std::unique_ptr<t>>
		{
			static constexpr bool is()
			{
				return true;
			}

			static std::string suffix()
			{
				return "-unique";
			}

			typedef t deref;

			static deref* get(std::unique_ptr<t>& a)
			{
				return a.get();
			}
		};

		template <typename t>
		struct type_wrap_info<std::shared_ptr<t>>
		{
			static constexpr bool is()
			{
				return true;
			}

			static std::string suffix()
			{
				return "-shared";
			}

			typedef t deref;

			static deref* get(std::shared_ptr<t>& a)
			{
				return a.get();
			}
		};

		template <typename t>
		struct type_wrap_info<std::optional<t>>
		{
			static constexpr bool is()
			{
				return true;
			}

			static std::string suffix()
			{
				return "-optional";
			}

			typedef t deref;

			static deref* get(std::optional<t>& a)
			{
				if (a)
				{
					return *a;
				}
				else
				{
					return nullptr;
				}
			}
		};

		template<typename t>
		class strong
		{
		private:
			std::remove_const_t<t> mutable val;
			
			strong() = delete;
			strong(strong<t> const&) = delete;
			void operator=(strong<t> const&) = delete;

		public:



			strong(t&& a) :
				val(std::move(a))
			{
				static_assert(std::is_trivially_destructible_v<t>, "strong should only be used for types that are trivially destructible");
			}

			strong(strong<t>&& a) :
				val(std::move(a.val))
			{}

			~strong()
			{
				//destructor needed to make this non trivially destructible
			}

			t& operator*() const
			{
				return val;
			}
		};

		template <typename t>
		struct type_wrap_info<strong<t>>
		{
			static constexpr bool is()
			{
				return true;
			}

			static std::string suffix()
			{
				return "-strong";
			}

			typedef t deref;

			static deref* get(strong<t>& a)
			{
				return &*a;
			}
		};

	}
}
