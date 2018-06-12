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
				return "-ptr";
			}

			typedef t wrapped;

			typedef t& deref;

			static bool has(t* const& a)
			{
				return a != nullptr;
			}

			static deref get(t*& a)
			{
				return *a;
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

			typedef t wrapped;

			typedef t& deref;

			static bool has(std::unique_ptr<t> const& a)
			{
				return a.get() != nullptr;
			}

			static deref get(std::unique_ptr<t>& a)
			{
				return *a;
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

			typedef t wrapped;

			typedef t& deref;

			static bool has(std::shared_ptr<t> const& a)
			{
				return a.get() != nullptr;
			}

			static deref get(std::shared_ptr<t>& a)
			{
				return *a;
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

			typedef t wrapped;

			typedef t& deref;

			static bool has(std::optional<t> const& a)
			{
				return a != std::nullopt;
			}

			static deref& get(std::optional<t>& a)
			{
				return *a
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

			std::remove_const_t<t>& operator*() const
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

			typedef t wrapped;

			typedef std::remove_const_t<t>& deref;

			static bool has(strong<t> const& a)
			{
				return true;
			}

			static deref& get(strong<t>& a)
			{
				return *a;
			}
		};

	}
}
