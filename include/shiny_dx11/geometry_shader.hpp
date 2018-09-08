#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny_dx11/d3d_fwd.hpp>

#include <atma/string.hpp>


namespace shiny_dx11
{
	struct geometry_shader_t
	{
		geometry_shader_t(d3d_blob_ptr const& blob, d3d_geometry_shader_ptr const& shader)
			: d3d_blob_{blob}
			, d3d_gs_{shader}
		{}

		auto d3d_blob() const -> d3d_blob_ptr const& { return d3d_blob_; }
		auto d3d_gs() const -> d3d_geometry_shader_ptr const& { return d3d_gs_; }

	private:
		d3d_blob_ptr d3d_blob_;
		d3d_geometry_shader_ptr d3d_gs_;
	};

	using geometry_shader_bridge_t = shiny::geometry_shader_bridge_t<geometry_shader_t>;
	using geometry_shader_bridge_ptr = atma::intrusive_ptr<geometry_shader_bridge_t>;
}

