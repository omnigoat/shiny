#pragma once

#include <shiny/shiny_fwd.hpp>

namespace shiny
{
	struct draw_target_t
	{
	
		auto render_target() const -> render_target_ptr const& { return render_target_; }
		auto depth_stencil_target() const -> depth_stencil_target_ptr const& { return depth_stencil_target_; }

	private:
		render_target_ptr render_target_;
		depth_stencil_target_ptr depth_stencil_target_;
	};
}
