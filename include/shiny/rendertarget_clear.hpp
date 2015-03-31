#pragma once

#include <atma/types.hpp>
#include <atma/math/vector4f.hpp>


namespace shiny
{
	struct rendertarget_clear_t
	{
		rendertarget_clear_t()
			: clear_color_{}, clear_depth_{}, clear_stencil_{}
		{}

		rendertarget_clear_t(bool clear_color, bool clear_depth, bool clear_stencil, aml::vector4f const& color = aml::vector4f{}, float depth = 1.f, uint8 stencil = 0)
			: clear_color_{clear_color}, clear_depth_{clear_depth}, clear_stencil_{clear_stencil}
			, color_{color}, depth_{depth}, stencil_{stencil}
		{}

		rendertarget_clear_t(aml::vector4f const& color)
			: rendertarget_clear_t{true, false, false, color}
		{}

		rendertarget_clear_t(aml::vector4f const& color, float depth)
			: rendertarget_clear_t{true, true, false, color, depth}
		{}

		auto clear_color() const -> bool { return clear_color_; }
		auto clear_depth() const -> bool { return clear_depth_; }
		auto clear_stencil() const -> bool { return clear_stencil_; }
		auto clear_any() const -> bool { return clear_color_ || clear_depth_ || clear_stencil_; }

		auto color() const -> aml::vector4f const& { return color_; }
		auto depth() const -> float { return depth_; }
		auto stencil() const -> uint8 { return stencil_; }

	private:
		bool clear_color_, clear_depth_, clear_stencil_;
		aml::vector4f color_;
		float depth_;
		uint8 stencil_;
	};
}
