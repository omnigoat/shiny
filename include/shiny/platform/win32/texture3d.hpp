#pragma once

#include <shiny/platform/win32/d3d_fwd.hpp>

#include <shiny/shiny_fwd.hpp>
#include <shiny/resource.hpp>
#include <shiny/element_format.hpp>

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>


namespace shiny
{
	struct texture3d_t : resource_t
	{
		friend auto create_texture3d(context_ptr const&, resource_usage_mask_t, element_format_t, size_t, size_t, size_t, uint) -> texture3d_ptr;

		auto format() const -> element_format_t;
		auto mips() const -> uint;
		auto width() const -> uint;
		auto height() const -> uint;
		auto depth() const -> uint;

		auto d3d_texture() const -> platform::d3d_texture3d_ptr const&;
		auto d3d_texture() -> platform::d3d_texture3d_ptr&;
		auto d3d_resource() const -> platform::d3d_resource_ptr override;

	private:
		texture3d_t(context_ptr const&, texture_usage_t, element_format_t, uint width, uint height, uint depth, uint mips);

	private:
		element_format_t format_;
		uint mips_;
		uint width_, height_, depth_;

		platform::d3d_texture3d_ptr d3d_texture_;
	};


	auto make_texture3d(
		context_ptr const&,
		resource_usage_mask_t,
		element_format_t,
		size_t width, size_t height, size_t depth, uint mips) -> texture3d_ptr;
}
