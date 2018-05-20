#pragma once
#include<memory>

namespace expr
{
	namespace impl
	{

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

			static std::string suffix()
			{
				return "-ref";
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

			static std::string suffix()
			{
				return "-unique";
			}

			typedef t deref;
		};

		template <typename t>
		struct pointer<std::shared_ptr<t>>
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

			/*t const& operator*() const
			{
				return val;
			}*/
		};

		template <typename t>
		struct pointer<strong<t>>
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
		};

	}
}