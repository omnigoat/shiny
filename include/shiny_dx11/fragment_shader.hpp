#pragma once

#include <shiny_dx11/d3d_fwd.hpp>

#include <shiny/shiny_fwd.hpp>
#include <shiny/fragment_shader.hpp>


namespace shiny_dx11
{
	struct fragment_shader_t
	{
		fragment_shader_t(d3d_blob_ptr const& blob, d3d_fragment_shader_ptr const& shader)
			: d3d_blob_{blob}
			, d3d_fs_{shader}
		{}

		auto d3d_blob() const -> d3d_blob_ptr const& { return d3d_blob_; }
		auto d3d_fs() const -> d3d_fragment_shader_ptr const& { return d3d_fs_; }

	private:
		d3d_blob_ptr d3d_blob_;
		d3d_fragment_shader_ptr d3d_fs_;
	};

	using fragment_shader_bridge_t = shiny::fragment_shader_bridge_t<fragment_shader_t>;
	using fragment_shader_bridge_ptr = atma::intrusive_ptr<fragment_shader_bridge_t>;
}

