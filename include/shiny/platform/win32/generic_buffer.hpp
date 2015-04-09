#pragma once

#include <shiny/buffer.hpp>
#include <shiny/platform/win32/dxgid3d_convert.hpp>


namespace shiny
{
	struct generic_buffer_t : shiny::buffer_t
	{
	protected:
		generic_buffer_t(context_ptr const&, resource_usage_mask_t const&, buffer_usage_t usage, element_format_t format, uint elements, void const* data, uint data_elemcount);
		generic_buffer_t(context_ptr const&, resource_usage_mask_t const&, buffer_usage_t usage, size_t stride, uint elements, void const* data, uint data_elemcount);

	private:
		friend struct context_t;
	};
}
