#pragma once

#include <shiny/platform/dx11/buffer.hpp>


namespace shiny
{
	struct vertex_buffer_t : buffer_t
	{
		vertex_buffer_t(renderer_ptr const&, resource_storage_t, data_declaration_t const*, uint vertex_count, void const* data, uint data_vertcount);
		~vertex_buffer_t();

		auto data_declaration() const -> data_declaration_t const* { return data_declaration_; }
		auto vertex_count() const -> uint { return vertex_count_; }

		auto sizeof_host_resource() const -> size_t override { return sizeof(vertex_buffer_t); }

	private:
		data_declaration_t const* data_declaration_;
		uint vertex_count_;
	};



	//
	// create_vertex_buffer
	// ----------------------
	//   if @data != nullptr, then @data_vertcount is the number of vertices that comprise
	//   the data. if @data_vertcount is zero, it assumes that there are @vertex_count
	//   vertices present (a.k.a: the buffer is to be totally filled). only if @data is
	//   not-null, of course.
	//
	inline auto create_vertex_buffer(
		renderer_ptr const& rndr, resource_storage_t usage,
		data_declaration_t const* vd, uint vertex_count,
		void const* data, uint data_vertcount = 0)
		-> vertex_buffer_ptr
	{
		return vertex_buffer_ptr(new vertex_buffer_t(rndr, usage, vd, vertex_count, data, data_vertcount));
	}
}

