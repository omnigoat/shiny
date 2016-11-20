#pragma once

#include <shiny/shiny_fwd.hpp>

#include <fooey/fooey_fwd.hpp>

#include <atma/config/platform.hpp>

namespace shiny
{
	uint32 const primary_adapter = 0;

	auto create_context(runtime_t&, fooey::window_ptr const&, uint32 adapter = primary_adapter) -> renderer_ptr;
}

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/win32/renderer.hpp>
#endif
