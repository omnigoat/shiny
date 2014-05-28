#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/resource.hpp>
#include <dust/platform/win32/d3d_fwd.hpp>

#include <atma/aligned_allocator.hpp>

#include <vector>
//======================================================================
namespace dust
{
	struct buffer_t : resource_t
	{
		typedef std::vector<char, atma::aligned_allocator_t<char, 4>> data_t;

		auto is_shadowing() const -> bool { return !shadow_buffer_.empty(); }
		auto usage() const -> buffer_usage_t { return usage_; }
		auto size() const -> size_t { return size_; }

		auto d3d_buffer() const -> platform::d3d_buffer_ptr const& { return d3d_buffer_; }

		auto d3d_resource() const -> platform::d3d_resource_ptr override;
		
	protected:
		buffer_t(context_ptr const&, buffer_type_t, buffer_usage_t, uint data_size, void const* data);
		virtual ~buffer_t();

		auto upload_shadow_buffer() -> void;

	protected:
		buffer_type_t type_;
		buffer_usage_t usage_;
		size_t size_;

		data_t shadow_buffer_;

		platform::d3d_buffer_ptr d3d_buffer_;


		friend auto create_buffer(context_ptr const&, buffer_type_t, buffer_usage_t, uint data_size, void* data) -> buffer_ptr;
	};
}
