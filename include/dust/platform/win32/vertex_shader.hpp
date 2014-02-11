#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/platform/win32/d3d_fwd.hpp>
//======================================================================
namespace dust {
	
	struct vertex_shader_t : atma::ref_counted
	{
		vertex_shader_t(context_ptr const&);

		auto d3d_vs() const -> platform::d3d_vertex_shader_ptr const& { return d3d_vs_; }
		auto d3d_blob() const -> platform::d3d_blob_ptr const& { return d3d_blob_; }

	private:
		context_ptr context_;

		platform::d3d_blob_ptr d3d_blob_;
		platform::d3d_vertex_shader_ptr d3d_vs_;
	};

}


