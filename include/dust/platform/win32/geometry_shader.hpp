#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/platform/win32/d3d_fwd.hpp>

#include <atma/unique_memory.hpp>
#include <atma/string.hpp>
//======================================================================
namespace dust
{
	struct geometry_stream_element_t
	{
		char const* tag;
		char start, count;
	};

	struct geometry_stream_t
	{
		std::vector<geometry_stream_element_t> elements;
	};



	struct geometry_shader_t : atma::ref_counted
	{
		friend struct context_t;
		friend auto create_vertex_shader(context_ptr const&, vertex_declaration_t const*, atma::unique_memory_t const&, bool precompiled, atma::string const& entrypoint = "main") -> vertex_shader_ptr;

		auto vertex_declaration() const -> vertex_declaration_t const* { return vertex_declaration_; }

		auto d3d_blob() const -> platform::d3d_blob_ptr const& { return d3d_blob_; }
		auto d3d_vs() const -> platform::d3d_vertex_shader_ptr const& { return d3d_gs_; }
		
	protected:
		geometry_shader_t(context_ptr const&, vertex_declaration_t const*, void const* data, size_t data_length, bool precompiled, atma::string const&);

	private:
		context_ptr context_;
		vertex_declaration_t const* vertex_declaration_;

		platform::d3d_blob_ptr d3d_blob_;
		platform::d3d_geometry_shader_ptr d3d_gs_;
		
	};
}


