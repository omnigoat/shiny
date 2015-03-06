#pragma once

#include <shiny/dust_fwd.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>


namespace shiny
{
	enum class blending_t : uint
	{
		zero,
		one,
		src_color,
		inv_src_color,
		src_alpha,
		inv_src_alpha,
		dest_color,
		inv_dest_color,
		dest_alpha,
		inv_dest_alpha,
	};

	struct rendertarget_blend_state_t
	{
		auto enable_blend     = true;

		auto dest_blend       = blending_t::inv_src_alpha;
		auto dest_blend_alpha = blending_t::inv_src_alpha;
		auto src_blend        = blending_t::src_alpha;
		auto src_blend_alpha  = blending_t::src_alpha;
	};

	struct multitarget_blend_state_t
	{
		render_target_blend_state_t render_target[8];
	};

	enum class depth_test_t
	{
		never,
		lt,
		lte,
		eq,
		neq,
		gte,
		gt,
		always
	};

	struct render_properties_t
	{
		
	};

	struct blend_state_t
	{
		blend_state_t(context_ptr const&, bool enable_blend, blending_t dest, blending_t dest_alpha, blending_t src, blending_t src_alpha)

		auto d3d_blend_state() const -> platform::d3d_blend_state_ptr const& { return d3d_blend_state_; }

	private:
		platform::d3d_blend_state_ptr d3d_blend_state_;
	};
}