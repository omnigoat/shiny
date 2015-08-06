#pragma once

#include <shiny/shiny_fwd.hpp>

#include <atma/intrusive_ptr.hpp>


namespace shiny
{
	struct draw_target_t : atma::ref_counted
	{
		draw_target_t(resource_view_ptr const&, resource_view_ptr const&);
		draw_target_t(resource_view_ptr const&);

		auto render_target() const -> resource_view_ptr const& { return render_target_; }
		auto depth_stencil() const -> resource_view_ptr const& { return depth_stencil_; }

	private:
		resource_view_ptr render_target_;
		resource_view_ptr depth_stencil_;
	};
}
