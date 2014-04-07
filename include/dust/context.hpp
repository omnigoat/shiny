#pragma once
//======================================================================
#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WIN32
#	include <dust/platform/win32/context.hpp>
#endif
//======================================================================
namespace dust
{
	struct runtime_t;

	auto create_context(runtime_t&, fooey::window_ptr const&, uint32 adapter = primary_adapter) -> context_ptr;
}

namespace dust_validation
{
	#pragma warning(disable: 4189)
	inline void context_t()
	{
		// context_t
		{ dust::context_t* _ = nullptr; }
		{ void (dust::context_t::*_)() = &dust::context_t::signal_block; }

		// create_context
		{ dust::context_ptr (*_)(dust::runtime_t&, fooey::window_ptr const&, uint32) = &dust::create_context; }
	}
}
