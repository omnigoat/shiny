#pragma once

#include <shiny/texture2d.hpp>

#include <shiny_dx11/d3d_fwd.hpp>


namespace shiny
{
	struct render_target_view_t : atma::ref_counted
	{
		render_target_view_t(texture2d_ptr const& tx, uint mip = 0);

		auto texture() const -> texture2d_ptr const& { return texture_; }
		auto mip() const -> uint { return mip_; }

	private:
		renderer_ptr ctx_;

		texture2d_ptr texture_;
		uint mip_;

		platform::d3d_render_target_view_ptr d3d_rt_;
	};
}
