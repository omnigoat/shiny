#pragma once

#include <shiny/dust_fwd.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <atma/unique_memory.hpp>
#include <atma/string.hpp>


namespace shiny
{
	struct geometry_shader_t : atma::ref_counted
	{
		auto d3d_blob() const -> platform::d3d_blob_ptr const& { return d3d_blob_; }
		auto d3d_gs() const -> platform::d3d_geometry_shader_ptr const& { return d3d_gs_; }
		
	protected:
		geometry_shader_t(context_ptr const&, void const* data, size_t data_length, bool precompiled, atma::string const&);

	private:
		context_ptr context_;

		platform::d3d_blob_ptr d3d_blob_;
		platform::d3d_geometry_shader_ptr d3d_gs_;


		friend struct context_t;
		friend auto create_geometry_shader(context_ptr const&, atma::unique_memory_t const&, bool precompiled, atma::string const& entrypoint = "main") -> geometry_shader_ptr;
	};
}

