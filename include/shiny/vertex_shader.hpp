#pragma once

#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/win32/vertex_shader.hpp>
#endif

namespace shiny {
	auto create_vertex_shader(context_ptr const&, atma::string const& path, bool precompiled, atma::string const& entrypoint = "main") -> vertex_shader_ptr;
}
