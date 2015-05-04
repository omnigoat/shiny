#pragma once

#include <shiny/buffer.hpp>
#include <shiny/platform/win32/dxgid3d_convert.hpp>


namespace shiny
{
	struct generic_buffer_t : shiny::buffer_t
	{
		generic_buffer_t(context_ptr const&, resource_usage_mask_t const&, resource_storage_t usage, size_t stride, uint elements, void const* data, uint data_elemcount);
	};

	auto make_generic_buffer(
		context_ptr const&,
		resource_usage_mask_t,
		resource_storage_t,
		size_t stride, uint elements,
		void const* data, uint data_elements) -> generic_buffer_ptr;
}
