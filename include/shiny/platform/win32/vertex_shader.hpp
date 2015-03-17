#pragma once

#include <shiny/dust_fwd.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <atma/unique_memory.hpp>
#include <atma/string.hpp>


namespace shiny
{
	struct vertex_shader_t : atma::ref_counted
	{
		friend struct context_t;
		friend auto create_vertex_shader(context_ptr const&, data_declaration_t const*, atma::unique_memory_t const&, bool precompiled, atma::string const& entrypoint = "main") -> vertex_shader_ptr;

		auto data_declaration() const -> data_declaration_t const* { return data_declaration_; }

		auto d3d_vs() const -> platform::d3d_vertex_shader_ptr const& { return d3d_vs_; }
		auto d3d_blob() const -> platform::d3d_blob_ptr const& { return d3d_blob_; }
		auto d3d_input_layout() const -> platform::d3d_input_layout_ptr const& { return d3d_input_layout_; }

	protected:
		vertex_shader_t(context_ptr const&, data_declaration_t const*, void const* data, size_t data_length, bool precompiled, atma::string const&);

	private:
		context_ptr context_;
		data_declaration_t const* data_declaration_;

		platform::d3d_blob_ptr d3d_blob_;
		platform::d3d_vertex_shader_ptr d3d_vs_;
		platform::d3d_input_layout_ptr d3d_input_layout_;
	};
}


