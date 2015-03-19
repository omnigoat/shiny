#pragma once
//======================================================================
#include <shiny/shiny_fwd.hpp>
#include <shiny/resource.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <atma/aligned_allocator.hpp>

#include <vector>
//======================================================================
namespace shiny
{
	struct buffer_t : resource_t
	{
		typedef std::vector<char, atma::aligned_allocator_t<char, 4>> data_t;

		auto type() const -> buffer_type_t { return type_; }
		auto usage() const -> buffer_usage_t { return usage_; }
		auto size() const -> size_t { return size_; }
		auto is_shadowing() const -> bool { return !shadow_buffer_.empty(); }

		auto d3d_buffer() const -> platform::d3d_buffer_ptr const& { return d3d_buffer_; }
		auto d3d_resource() const -> platform::d3d_resource_ptr override;
		auto d3d_srv() const -> platform::d3d_shader_resource_view_ptr const& override;

	protected:
		buffer_t(context_ptr const&, buffer_type_t, buffer_usage_t, uint element_size, uint element_count, void const* data, uint data_element_count);
		virtual ~buffer_t();

		auto upload_shadow_buffer() -> void;

	protected:
		buffer_type_t type_;
		buffer_usage_t usage_;
		size_t size_;

		data_t shadow_buffer_;

		platform::d3d_buffer_ptr d3d_buffer_;
		platform::d3d_shader_resource_view_ptr d3d_srv_;
	};
}
