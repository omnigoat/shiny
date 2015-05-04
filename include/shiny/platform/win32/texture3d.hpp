#pragma once

#include <shiny/platform/win32/d3d_fwd.hpp>

#include <shiny/shiny_fwd.hpp>
#include <shiny/resource.hpp>
#include <shiny/element_format.hpp>

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>


namespace shiny
{
	struct texture3d_dimensions_t
	{
		texture3d_dimensions_t(element_format_t format, size_t width, size_t height, size_t depth, uint mips)
			: format(format), width(width), height(height), depth(depth), mips(mips)
		{}

		static auto cube(element_format_t format, size_t size, uint mips) -> texture3d_dimensions_t
		{
			return texture3d_dimensions_t{format, size, size, size, mips};
		}

		element_format_t format;
		size_t width, height, depth;
		uint mips;
	};



	struct texture3d_t : resource_t
	{
		texture3d_t(context_ptr const&, resource_usage_mask_t, resource_storage_t, texture3d_dimensions_t const&);

		auto format() const -> element_format_t;
		auto mips() const -> uint;
		auto width() const -> size_t;
		auto height() const -> size_t;
		auto depth() const -> size_t;

		auto d3d_texture() const -> platform::d3d_texture3d_ptr const&;
		auto d3d_texture() -> platform::d3d_texture3d_ptr&;
		auto d3d_resource() const -> platform::d3d_resource_ptr override;

	private:
		element_format_t format_;
		uint mips_;
		size_t width_, height_, depth_;

		platform::d3d_texture3d_ptr d3d_texture_;
	};


	auto make_texture3d(
		context_ptr const&,
		resource_usage_mask_t,
		resource_storage_t,
		texture3d_dimensions_t const&) -> texture3d_ptr;
}
