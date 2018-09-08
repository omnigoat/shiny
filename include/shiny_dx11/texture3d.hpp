#pragma once

#include <shiny_dx11/d3d_fwd.hpp>
#include <shiny_dx11/resource.hpp>

#include <shiny/shiny_fwd.hpp>
#include <shiny/format.hpp>
#include <shiny/texture3d.hpp>

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>


namespace shiny_dx11
{
	struct texture3d_t : resource_t
	{
		texture3d_t(shiny::renderer_ptr const&,
			shiny::resource_usage_mask_t, shiny::resource_storage_t,
			shiny::format_t format, size_t width, size_t height, size_t depth, uint mips);

		auto d3d_texture() const -> d3d_texture3d_ptr const& { return d3d_texture_; }
		auto d3d_resource() const -> d3d_resource_ptr override { return d3d_texture_; }

	private:
		d3d_texture3d_ptr d3d_texture_;
	};

	using texture3d_bridge_t = shiny::texture3d_bridge_t<shiny_dx11::texture3d_t>;
	using texture3d_bridge_ptr = atma::intrusive_ptr<texture3d_bridge_t>;
}
