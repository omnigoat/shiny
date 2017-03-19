#pragma once

#include <shiny/platform/dx11/d3d_fwd.hpp>

#include <shiny/shiny_fwd.hpp>
#include <shiny/resource.hpp>
#include <shiny/resource_view.hpp>
#include <shiny/format.hpp>

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>


namespace shiny_d3d11
{
	struct texture2d_dx11_t : shiny::resource_t
	{
		texture2d_dx11_t(renderer_ptr const&, resource_usage_mask_t, format_t, uint width, uint height, uint mips);

		auto d3d_texture() const -> platform::d3d_texture2d_ptr const& { return d3d_texture_; }
		auto d3d_texture() -> platform::d3d_texture2d_ptr& { return d3d_texture_; }
		auto d3d_resource() const -> platform::d3d_resource_ptr override { return d3d_texture_; }

	private:
		texture2d_dx11_t(renderer_ptr const&, platform::d3d_texture2d_ptr const&, resource_usage_mask_t, format_t, uint width, uint height, uint mips);

	private:
		platform::d3d_texture2d_ptr d3d_texture_;

		friend struct renderer_t;
	};
}


