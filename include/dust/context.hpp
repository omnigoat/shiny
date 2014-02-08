#pragma once
//======================================================================
#include <dust/runtime.hpp>

#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WIN32
#	include <dust/platform/win32/context.hpp>
#endif
//======================================================================
namespace dust
{
	auto create_context(runtime_t&, fooey::window_ptr const&, uint32_t adapter = primary_adapter) -> context_ptr;
}
