#pragma once
//======================================================================
#include <shiny/dust_fwd.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <atma/unique_memory.hpp>
#include <atma/string.hpp>
//======================================================================
namespace shiny
{
	struct fragment_shader_t : atma::ref_counted
	{
		friend auto create_fragment_shader(context_ptr const&, atma::unique_memory_t const&, bool precompiled, atma::string const& entrypoint = "main") -> fragment_shader_ptr;

		auto d3d_ps() const -> platform::d3d_fragment_shader_ptr const& { return d3d_ps_; }

	private:
		fragment_shader_t(context_ptr const&, void const*, size_t, bool, atma::string const&);

	private:
		context_ptr context_;

		platform::d3d_blob_ptr d3d_blob_;
		platform::d3d_fragment_shader_ptr d3d_ps_;
	};
}


