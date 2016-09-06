#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <atma/string.hpp>


namespace shiny
{
	struct fragment_shader_t : atma::ref_counted
	{
		auto d3d_blob() const -> platform::d3d_blob_ptr const& { return d3d_blob_; }
		auto d3d_fs() const -> platform::d3d_fragment_shader_ptr const& { return d3d_fs_; }

	protected:
		fragment_shader_t(context_ptr const&, atma::string const&, void const*, size_t, bool, atma::string const&);

	private:
		context_ptr context_;
		atma::string path_;

		platform::d3d_blob_ptr d3d_blob_;
		platform::d3d_fragment_shader_ptr d3d_fs_;


		friend atma::enable_intrusive_ptr_make;
	};
}


