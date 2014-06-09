#pragma once
//======================================================================
#include <dust/platform/win32/buffer.hpp>
//======================================================================
namespace dust
{
	struct vertex_buffer_t : buffer_t
	{
		vertex_buffer_t(context_ptr const&, buffer_usage_t, vertex_declaration_t const&, uint vertex_count, void* data);
		~vertex_buffer_t();

		auto vertex_count() const -> uint { return vertex_count_; }

	private:
		uint vertex_count_;
	};


	inline auto create_vertex_buffer(context_ptr const& ctx, buffer_usage_t usage, vertex_declaration_t const& vd, uint vertex_count, void* data) -> vertex_buffer_ptr {
		return vertex_buffer_ptr(new vertex_buffer_t(ctx, usage, vd, vertex_count, data));
	}
}

