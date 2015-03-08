#pragma once

#include <shiny/dust_fwd.hpp>
#include <shiny/blend_state.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>


namespace shiny
{
	
	enum class comparison_t
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

	enum class stencil_op_t
	{
		keep,
		zero,
		inc,
		dec,
		inc_clamp,
		dec_clamp,
		invert,
	};

	

	struct depth_stencil_state_t
	{
		bool depth_enabled         = true;
		bool depth_write_enabled   = true;
		comparison_t depth_test    = comparison_t::lt;

		bool stencil_enabled       = false;
		comparison_t stencil_test  = comparison_t::always;

		// ff: front-facing, bf: back-facing
		stencil_op_t stencil_ff_pass_op       = stencil_op_t::keep;
		stencil_op_t stencil_ff_fail_op       = stencil_op_t::keep;
		stencil_op_t stencil_ff_depth_fail_op = stencil_op_t::keep;
		stencil_op_t stencil_bf_pass_op       = stencil_op_t::keep;
		stencil_op_t stencil_bf_fail_op       = stencil_op_t::keep;
		stencil_op_t stencil_bf_depth_fail_op = stencil_op_t::keep;
	};




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