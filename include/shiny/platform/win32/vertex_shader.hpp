#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <atma/string.hpp>


namespace shiny
{
	struct vertex_shader_t : atma::ref_counted
	{
		auto d3d_blob() const -> platform::d3d_blob_ptr const& { return d3d_blob_; }
		auto d3d_vs() const -> platform::d3d_vertex_shader_ptr const& { return d3d_vs_; }

	protected:
		vertex_shader_t(context_ptr const&, atma::string const&, void const*, size_t, bool, atma::string const&);

	private:
		context_ptr context_;
		atma::string path_;

		platform::d3d_blob_ptr d3d_blob_;
		platform::d3d_vertex_shader_ptr d3d_vs_;


		friend struct atma::enable_intrusive_ptr_make;
	};
}


