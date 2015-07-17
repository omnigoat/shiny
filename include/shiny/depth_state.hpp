#pragma once

namespace shiny
{
	enum class stencil_op_t
	{
		keep,
		zero,
		inc,
		dec,
		inc_clamp,
		dec_clamp,
		replace,
		invert,
	};

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

	struct depth_stencil_state_t
	{
		depth_stencil_state_t()
		{}

		depth_stencil_state_t(bool enabled)
			: depth_enabled(enabled)
		{}

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

		static depth_stencil_state_t const off;
		static depth_stencil_state_t const standard;
	};

	inline auto operator == (depth_stencil_state_t const& lhs, depth_stencil_state_t const& rhs) -> bool
	{
		return memcmp(&lhs, &rhs, sizeof(depth_stencil_state_t)) == 0;
	}

}
