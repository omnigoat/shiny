#pragma once
//======================================================================
#include <shiny/buffer.hpp>
//======================================================================
namespace shiny
{
	struct index_buffer_t : buffer_t
	{
		index_buffer_t(context_ptr const&, buffer_usage_t, uint index_size, uint index_count, void const* data, uint data_indexcount);
		~index_buffer_t();

		auto index_count() const -> uint { return index_count_; }

	private:
		uint index_count_;

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
		context_ptr const& ctx, buffer_usage_t usage,
		uint index_size, uint index_count,
		void const* data, uint data_indexcount = 0)
		-> index_buffer_ptr
	{
		return index_buffer_ptr(new index_buffer_t(ctx, usage, index_size, index_count, data, data_indexcount));
	}
}
