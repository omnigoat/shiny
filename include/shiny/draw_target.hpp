#pragma once

#include <shiny/shiny_fwd.hpp>

#include <atma/intrusive_ptr.hpp>


namespace shiny
{
	struct draw_target_t : atma::ref_counted
	{
		draw_target_t(render_target_view_ptr const&, depth_stencil_view_ptr const&);
		draw_target_t(render_target_view_ptr const&);

		auto render_target() const -> render_target_view_ptr const& { return render_target_; }
		auto depth_stencil_target() const -> depth_stencil_view_ptr const& { return depth_stencil_target_; }

	private:
		render_target_view_ptr render_target_;
		depth_stencil_view_ptr depth_stencil_target_;
	};
}
