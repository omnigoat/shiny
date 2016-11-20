#pragma once

#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/win32/geometry_shader.hpp>
#endif

namespace shiny {
	auto create_geometry_shader(renderer_ptr const&, atma::string const& path, bool precompiled, atma::string const& entrypoint = "main") -> geometry_shader_ptr;
}
