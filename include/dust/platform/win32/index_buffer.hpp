#pragma once
//======================================================================
#include <dust/adapter.hpp>
#include <dust/dust_fwd.hpp>
#include <dust/platform/win32/d3d_fwd.hpp>

#include <atma/aligned_allocator.hpp>

#include <vector>
//======================================================================
namespace dust {
//======================================================================

	struct index_buffer_t : atma::ref_counted
	{
		typedef std::vector<char, atma::aligned_allocator_t<char, 4>> data_t;

		index_buffer_t(context_ptr const&, buffer_usage_t, uint index_size, uint index_count, void* data);
		~index_buffer_t();

		auto usage() const -> buffer_usage_t { return usage_; }
		auto is_shadowing() const -> bool;
		auto capacity() const -> size_t;
		auto size() const -> size_t;
		auto index_count() const -> uint { return index_count_; }
		auto d3d_buffer() const -> platform::d3d_buffer_ptr const& { return d3d_buffer_; }

	private:
		auto upload_shadow_buffer() -> void;

		buffer_usage_t usage_;
		gpu_access_t gpu_access_;
		cpu_access_t cpu_access_;

		uint index_count_;
		size_t capacity_;
		size_t size_;
		data_t shadow_buffer_;

		context_ptr context_;
		platform::d3d_buffer_ptr d3d_buffer_;

		friend struct locked_index_buffer_t;
	};


	template <typename... Args>
	auto create_index_buffer(Args&&... args) -> index_buffer_ptr
	//auto create_index_buffer(context_ptr const&, buffer_usage_t, uint data_size, void* data) -> index_buffer_ptr
	{
		return index_buffer_ptr(new index_buffer_t(std::forward<Args>(args)...));
	}

//======================================================================
} // namespace dust
//======================================================================
