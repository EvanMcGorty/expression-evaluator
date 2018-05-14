#pragma once
#include<memory>

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
	mutable t val;
public:
	strong(t&& a)
	{
		val = std::move(a);
	}
	
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