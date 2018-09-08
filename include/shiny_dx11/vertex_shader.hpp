#pragma once

#include <shiny_dx11/d3d_fwd.hpp>

#include <shiny/shiny_fwd.hpp>
#include <shiny/vertex_shader.hpp>


namespace shiny_dx11
{
	struct vertex_shader_t
	{
		vertex_shader_t(d3d_blob_ptr const& blob, d3d_vertex_shader_ptr const& shader)
			: d3d_blob_{blob}
			, d3d_vs_{shader}
		{}

		auto d3d_blob() const -> d3d_blob_ptr const& { return d3d_blob_; }
		auto d3d_vs() const -> d3d_vertex_shader_ptr const& { return d3d_vs_; }

	private:
		d3d_blob_ptr d3d_blob_;
		d3d_vertex_shader_ptr d3d_vs_;
	};

	using vertex_shader_bridge_t = shiny::vertex_shader_bridge_t<vertex_shader_t>;
	using vertex_shader_bridge_ptr = atma::intrusive_ptr<vertex_shader_bridge_t>;
}

