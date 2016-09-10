#pragma once

#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/win32/fragment_shader.hpp>
#endif

#include <lion/streams.hpp>


// implemented per-platform
namespace shiny
{
	// from file
	auto create_fragment_shader(
		context_ptr const&,
		atma::string const& path,
		bool precompiled,
		atma::string const& entrypoint = "main") -> fragment_shader_ptr;

	// from file opened via stream
	auto create_fragment_shader(
		context_ptr const&,
		atma::string const& path,
		lion::input_stream_ptr const&,
		bool precompiled,
		atma::string const& entrypoint = "main") -> fragment_shader_ptr;
}
