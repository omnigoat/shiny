#pragma once
//======================================================================
#include <dust/buffer.hpp>
#include <dust/platform/win32/dxgid3d_convert.hpp>
//======================================================================
namespace dust
{
	struct generic_buffer_t : dust::buffer_t
	{
		generic_buffer_t(context_ptr const& ctx, buffer_usage_t usage, element_format_t format, uint elements, void const* data, uint data_elemcount);

		// todo: constructor that shares the data of another genric-buffer but has a different view

		auto element_count() const -> uint { return element_count_; }

		auto d3d_shader_resource_view() const -> platform::d3d_shader_resource_view_ptr const& { return d3d_srv_; }

	private:
		uint element_count_;

		platform::d3d_shader_resource_view_ptr d3d_srv_;
	};

	inline auto create_generic_buffer(
		context_ptr const& ctx, buffer_usage_t usage, element_format_t format,
		uint elements, void const* data, uint data_elemcount)
	-> generic_buffer_ptr
	{
		return generic_buffer_ptr(new generic_buffer_t(ctx, usage, format, elements, data, data_elemcount));
	}
}