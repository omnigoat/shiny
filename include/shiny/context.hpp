#pragma once
//======================================================================
#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WIN32
#	include <shiny/platform/win32/context.hpp>
#endif
//======================================================================
namespace shiny
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
		{ shiny::context_t* _ = nullptr; }
		{ void (shiny::context_t::*_)() = &shiny::context_t::signal_block; }

		// create_context
		{ shiny::context_ptr (*_)(shiny::runtime_t&, fooey::window_ptr const&, uint32) = &shiny::create_context; }
	}
}
