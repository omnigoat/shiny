#pragma once

#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WIN32
#	include <dust/platform/win32/compute_shader.hpp>
#endif

// implemented per-platform
namespace dust {
	auto create_compute_shader(context_ptr const&, void const* data, uint data_size) -> compute_shader_ptr;
}
