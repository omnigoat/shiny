#pragma once

#include <shiny/buffer.hpp>
#include <shiny/format.hpp>

namespace shiny
{
	struct index_buffer_t : buffer_t
	{
		index_buffer_t(renderer_ptr const&, resource_storage_t, format_t, uint index_count, void const* data, uint data_indexcount);
		~index_buffer_t();

		auto index_count() const -> uint { return index_count_; }
		auto index_format() const -> format_t { return index_format_; }

		auto sizeof_host_resource() const -> size_t override { return sizeof(index_buffer_t); }

	private:
		uint index_count_;
		format_t index_format_;

		friend struct locked_index_buffer_t;
	};



	//
	// create_index_buffer
	// ----------------------
	//   if @data != nullptr, then @data_indexcount is the number of vertices that comprise
	//   the data. if @data_indexcount is zero, it assumes that there are @vertex_count
	//   vertices present (a.k.a: the buffer is to be totally filled). only if @data is
	//   not-null, of course.
	//
	inline auto create_index_buffer(
		renderer_ptr const& rndr, resource_storage_t usage,
		format_t format, uint index_count,
		void const* data, uint data_indexcount = 0)
		-> index_buffer_ptr
	{
		return index_buffer_ptr(new index_buffer_t(rndr, usage, format, index_count, data, data_indexcount));
	}
}
