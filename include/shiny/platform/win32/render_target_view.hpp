#pragma once

#include <shiny/texture2d.hpp>

#include <shiny/platform/win32/d3d_fwd.hpp>


namespace shiny
{
	struct render_target_view_t
	{
		render_target_view_t(texture2d_ptr const& tx, uint mip = 0);

	private:
		context_ptr ctx_;

		texture2d_ptr texture_;
		uint mip_;

		platform::d3d_render_target_view_ptr d3d_rt_;
	};
}
