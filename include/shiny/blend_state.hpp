#pragma once

namespace shiny
{
	enum class blending_t : uint
	{
		zero,
		one,
		src_color,
		one_minus_src_color,
		src_alpha,
		one_minus_src_alpha,
		dest_color,
		one_minus_dest_color,
		dest_alpha,
		one_minus_dest_alpha,
	};

	struct rendertarget_blend_state_t
	{
		bool blending_enabled = true;

		blending_t dest_blend       = blending_t::one_minus_src_alpha;
		blending_t dest_blend_alpha = blending_t::zero;
		blending_t src_blend        = blending_t::src_alpha;
		blending_t src_blend_alpha  = blending_t::zero;
	};

	inline auto operator == (rendertarget_blend_state_t const& lhs, rendertarget_blend_state_t const& rhs) -> bool
	{
		return lhs.blending_enabled == rhs.blending_enabled &&
			lhs.dest_blend == rhs.dest_blend &&
			lhs.dest_blend_alpha == rhs.dest_blend_alpha &&
			lhs.src_blend == rhs.src_blend &&
			lhs.src_blend_alpha == rhs.src_blend_alpha;
	}




	struct blend_state_t
	{
		blend_state_t(blending_t dest_blend, blending_t dest_blend_alpha, blending_t src_blend, blending_t src_blend_alpha)
			: multitarget_collapse(true)
		{
			rendertarget[0].blending_enabled = true;
			rendertarget[0].dest_blend = dest_blend;
			rendertarget[0].dest_blend_alpha = dest_blend_alpha;
			rendertarget[0].src_blend = src_blend;
			rendertarget[0].src_blend_alpha = src_blend_alpha;
		}

		static auto opaque() -> blend_state_t
		{
			return blend_state_t{blending_t::one_minus_src_alpha, blending_t::zero, blending_t::src_alpha, blending_t::zero};
		}

		static auto transparent() -> blend_state_t
		{
			return blend_state_t{blending_t::one_minus_src_alpha, blending_t::zero, blending_t::src_alpha, blending_t::one};
		}

		// all targets use first target's properties
		bool multitarget_collapse = false;

		rendertarget_blend_state_t rendertarget[8];
	};

	inline auto operator == (blend_state_t const& lhs, blend_state_t const& rhs) -> bool
	{
		return lhs.multitarget_collapse == rhs.multitarget_collapse &&
			lhs.rendertarget[0] == rhs.rendertarget[0] &&
			lhs.rendertarget[1] == rhs.rendertarget[1] &&
			lhs.rendertarget[2] == rhs.rendertarget[2] &&
			lhs.rendertarget[3] == rhs.rendertarget[3] &&
			lhs.rendertarget[4] == rhs.rendertarget[4] &&
			lhs.rendertarget[5] == rhs.rendertarget[5] &&
			lhs.rendertarget[6] == rhs.rendertarget[6] &&
			lhs.rendertarget[7] == rhs.rendertarget[7];
	}
}


