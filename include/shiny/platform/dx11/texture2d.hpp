#pragma once

#include <shiny/platform/dx11/d3d_fwd.hpp>
#include <shiny/platform/dx11/resource.hpp>

#include <shiny/shiny_fwd.hpp>
#include <shiny/texture2d.hpp>
#include <shiny/resource_view.hpp>
#include <shiny/format.hpp>

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>


namespace shiny_dx11
{
	struct texture2d_t : resource_dx11_t
	{
		texture2d_t(shiny::renderer_ptr const&, shiny::resource_usage_mask_t, shiny::format_t, uint width, uint height, uint mips);
		texture2d_t(shiny::renderer_ptr const&, d3d_texture2d_ptr const&, shiny::resource_usage_mask_t, shiny::format_t, uint width, uint height, uint mips);

		auto d3d_texture() const -> d3d_texture2d_ptr const& { return d3d_texture_; }
		auto d3d_texture() -> d3d_texture2d_ptr& { return d3d_texture_; }
		auto d3d_resource() const -> d3d_resource_ptr override { return d3d_texture_; }

	private:
		d3d_texture2d_ptr d3d_texture_;

		friend struct shiny::renderer_t;
	};

	using texture2d_bridge_t = shiny::texture2d_bridge_t<shiny_dx11::texture2d_t>;
}

