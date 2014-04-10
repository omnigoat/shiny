#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/adapter.hpp>
#include <dust/platform/win32/buffer.hpp>
#include <dust/platform/win32/d3d_fwd.hpp>

#include <atma/assert.hpp>
#include <atma/aligned_allocator.hpp>

#include <vector>
//======================================================================
namespace dust
{
	struct vertex_buffer_t : buffer_t
	{
		auto vertex_count() const -> uint { return vertex_count_; }

	private:
		vertex_buffer_t(context_ptr const&, buffer_usage_t, vertex_declaration_t const&, uint vertex_count, void* data);
		~vertex_buffer_t();

	private:
		uint vertex_count_;
		size_t size_;


		friend auto create_vertex_buffer(context_ptr const&, buffer_usage_t, vertex_declaration_t const&, uint vertex_count, void* data) -> vertex_buffer_ptr;
	};
}

