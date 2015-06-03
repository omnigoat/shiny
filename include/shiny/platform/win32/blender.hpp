#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/blend_state.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>


namespace shiny
{
	struct blender_t : atma::ref_counted
	{
		auto d3d_blend_state() const -> platform::d3d_blend_state_ptr const& { return d3d_blend_state_; }

	private:
		blender_t(context_ptr const&, platform::d3d_blend_state_ptr const& bs)
			: d3d_blend_state_(bs)
		{}

	private:
		platform::d3d_blend_state_ptr d3d_blend_state_;

		friend struct context_t;
	};

}
