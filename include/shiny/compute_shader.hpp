#pragma once

#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/win32/compute_shader.hpp>
#endif

// implemented per-platform
namespace shiny {
	auto create_compute_shader(context_ptr const&, atma::string const& path, bool precompiled, atma::string const& entrypoint = "main") -> compute_shader_ptr;
}
