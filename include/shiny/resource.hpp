#pragma once
//======================================================================
#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/dx11/resource.hpp>
#endif

namespace shiny
{
	template <typename C, typename I>
	inline constexpr C* inteface_to_concrete(I* i)
	{
		return reinterpret_cast<C*>((char*)i + sizeof(I));
	}

	template <typename I, typename C>
	inline constexpr C* concrete_to_interface(C* c)
	{
		return reinterpret_cast<I*>((char*)c - sizeof(I));
	}

	template <class Interface, class Concrete>
	struct resource_bridge_t
		: Interface
	{
		resource_bridge_t(Interface&& i, Concrete&& c)
			: Interface{std::move(i)}
			, concrete_{std::move(c)}
		{}

	protected:
		Concrete concrete_;
	};
}
