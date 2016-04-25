#pragma once

#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/win32/compute_shader.hpp>
#endif

// implemented per-platform
namespace shiny {
	auto make_compute_shader(context_ptr const&, void const* data, size_t data_size) -> compute_shader_ptr;
}
