#pragma once

#include <shiny/platform/win32/d3d_fwd.hpp>

#include <shiny/shiny_fwd.hpp>
#include <shiny/resource.hpp>
#include <shiny/resource_view.hpp>
#include <shiny/format.hpp>

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>


namespace shiny
{
	struct texture2d_t : resource_t
	{
		texture2d_t(renderer_ptr const&, resource_usage_mask_t, format_t, uint width, uint height, uint mips);
		texture2d_t(renderer_ptr const&, resource_usage_mask_t, format_t, uint width, uint height, uint mips, resource_subset_t);

		auto format() const -> format_t;
		auto width() const -> uint;
		auto height() const -> uint;
		auto mips() const -> uint;

		auto d3d_texture() const -> platform::d3d_texture2d_ptr const&;
		auto d3d_texture() -> platform::d3d_texture2d_ptr&;
		auto d3d_resource() const -> platform::d3d_resource_ptr override;

	private:
		texture2d_t(renderer_ptr const&, platform::d3d_texture2d_ptr const&, resource_usage_mask_t, format_t, uint width, uint height, uint mips);

	private:
		format_t format_;
		uint width_, height_;
		uint mips_;

		platform::d3d_texture2d_ptr d3d_texture_;

		friend struct renderer_t;
	};


	auto make_texture2d(renderer_ptr const&, resource_usage_mask_t, format_t, uint width, uint height) -> texture2d_ptr;
}


