#pragma once
//======================================================================
#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/dx11/runtime.hpp>
#endif

#include <shiny/renderer.hpp>

#if 0
namespace shiny
{
	struct output_t : device_interop_t
	{
		auto sizeof_host_resource() const -> size_t override { return sizeof(output_t); }
	};

	using output_ptr = atma::intrusive_ptr<output_t>;
}
#endif

#if 0
namespace shiny
{
	struct runtime_t
	{
		auto output_for_window(fooey::window_ptr const&) -> output_ptr = 0;
		auto create_renderer() const -> renderer_ptr;
	};
}
#endif