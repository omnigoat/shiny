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
