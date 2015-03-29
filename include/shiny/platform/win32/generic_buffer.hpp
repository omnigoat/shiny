#pragma once
//======================================================================
#include <shiny/buffer.hpp>
#include <shiny/platform/win32/dxgid3d_convert.hpp>
//======================================================================
namespace shiny
{
	struct generic_buffer_t : shiny::buffer_t
	{
		generic_buffer_t(context_ptr const& ctx, buffer_usage_t usage, element_format_t format, uint elements, void const* data, uint data_elemcount);
		generic_buffer_t(context_ptr const& ctx, buffer_usage_t usage, size_t stride, uint elements, void const* data, uint data_elemcount);

		// todo: constructor that shares the data of another genric-buffer but has a different view

		auto element_count() const -> uint { return element_count_; }

	private:
		uint element_count_;
	};




	inline auto create_generic_buffer(
		context_ptr const& ctx, buffer_usage_t usage, element_format_t format,
		uint elements, void const* data, uint data_elemcount)
	-> generic_buffer_ptr
	{
		return generic_buffer_ptr(new generic_buffer_t(ctx, usage, format, elements, data, data_elemcount));
	}

	inline auto create_generic_buffer(
		context_ptr const& ctx, buffer_usage_t usage, size_t stride,
		uint elements, void const* data, uint data_elemcount)
		-> generic_buffer_ptr
	{
		return generic_buffer_ptr(new generic_buffer_t(ctx, usage, stride, elements, data, data_elemcount));
	}
}