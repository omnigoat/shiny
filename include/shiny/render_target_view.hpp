#pragma once

#include <shiny/shiny_fwd.hpp>

#include <atma/config/platform.hpp>

namespace shiny
{
	auto make_render_target_view(texture2d_ptr const&) -> render_target_view_ptr;
}

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/dx11/render_target_view.hpp>
#endif
