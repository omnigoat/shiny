#pragma once

#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WIN32
#	include <dust/platform/win32/vertex_shader.hpp>
#endif

// implemented per-platform
namespace dust {
	auto create_vertex_shader(context_ptr const&) -> vertex_shader_ptr;
}
