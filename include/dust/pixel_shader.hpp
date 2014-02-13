#pragma once

#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WIN32
#	include <dust/platform/win32/pixel_shader.hpp>
#endif

// implemented per-platform
namespace dust {
	auto create_pixel_shader(context_ptr const&) -> pixel_shader_ptr;
}
