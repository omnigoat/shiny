#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/platform/win32/d3d_fwd.hpp>

#include <atma/assert.hpp>
#include <atma/aligned_allocator.hpp>

#include <vector>
//======================================================================
namespace dust
{
	struct buffer_t : atma::ref_counted
	{
		typedef std::vector<char, atma::aligned_allocator_t<char, 4>> data_t;

		auto usage() const -> buffer_usage_t { return usage_; }
		auto size() const -> size_t { return size_; }
		auto d3d_buffer() const -> platform::d3d_buffer_ptr const& { return d3d_buffer_; }

	private:
		buffer_t(context_ptr const&, buffer_usage_t, vertex_declaration_t const&, uint vertex_count, void* data);
		~buffer_t();

		auto upload_shadow_buffer() -> void;

	private:
		context_ptr context_;

		buffer_usage_t usage_;
		gpu_access_t gpu_access_;
		cpu_access_t cpu_access_;
		size_t size_;

		platform::d3d_buffer_ptr d3d_buffer_;

		friend auto create_buffer(context_ptr const&, buffer_usage_t, vertex_declaration_t const&, uint32 vertex_count, void* data) -> vertex_buffer_ptr;
	};
}
