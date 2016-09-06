#pragma once

#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/win32/fragment_shader.hpp>
#endif

// implemented per-platform
namespace shiny {
	auto create_fragment_shader(context_ptr const&, atma::string const& path, bool precompiled, atma::string const& entrypoint = "main") -> fragment_shader_ptr;
}
