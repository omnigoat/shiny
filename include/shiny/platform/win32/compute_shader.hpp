#pragma once
//======================================================================
#include <shiny/dust_fwd.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <atma/types.hpp>
//======================================================================
namespace shiny
{
	struct compute_shader_t : atma::ref_counted
	{
		compute_shader_t(context_ptr const&, void const* data, size_t data_size);

		auto d3d_cs() const -> platform::d3d_compute_shader_ptr const& { return d3d_cs_; }
		auto d3d_blob() const -> platform::d3d_blob_ptr const& { return d3d_blob_; }

	private:
		context_ptr context_;

		platform::d3d_blob_ptr d3d_blob_;
		platform::d3d_compute_shader_ptr d3d_cs_;
	};
}


