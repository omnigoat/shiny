#pragma once

#include <shiny/buffer.hpp>
#include <shiny/platform/dx11/dxgid3d_convert.hpp>


namespace shiny
{
	struct generic_buffer_t : shiny::buffer_t
	{
		generic_buffer_t(renderer_ptr const&, resource_usage_mask_t const&, resource_storage_t usage, size_t stride, uint elements, void const* data, uint data_elemcount);

		auto sizeof_host_resource() const -> size_t override { return sizeof(generic_buffer_t); }
	};

	auto make_generic_buffer(
		renderer_ptr const&,
		resource_usage_mask_t,
		resource_storage_t,
		size_t stride, uint elements,
		void const* data, uint data_elements) -> generic_buffer_ptr;
}
