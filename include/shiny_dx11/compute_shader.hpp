#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny_dx11/d3d_fwd.hpp>

#include <atma/string.hpp>


namespace shiny
{
	struct compute_shader_t : atma::ref_counted
	{
		auto d3d_blob() const -> platform::d3d_blob_ptr const& { return d3d_blob_; }
		auto d3d_cs() const -> platform::d3d_compute_shader_ptr const& { return d3d_cs_; }

	protected:
		compute_shader_t(renderer_ptr const&, atma::string const&, void const*, size_t, bool, atma::string const&);

	private:
		renderer_ptr rndr_;
		atma::string path_;

		platform::d3d_blob_ptr d3d_blob_;
		platform::d3d_compute_shader_ptr d3d_cs_;


		friend struct atma::enable_intrusive_ptr_make;
	};
}


