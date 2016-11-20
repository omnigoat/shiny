#pragma once

#include <shiny/buffer.hpp>
#include <shiny/platform/win32/dxgid3d_convert.hpp>


namespace shiny
{
	struct structured_buffer_t : shiny::buffer_t
	{
		structured_buffer_t(renderer_ptr const&, resource_usage_mask_t const&, resource_storage_t usage, size_t stride, uint elements, void const* data, uint data_elemcount);
	};

	auto make_structured_buffer(
		renderer_ptr const&,
		resource_usage_mask_t,
		resource_storage_t,
		size_t stride, uint elements,
		void const* data, uint data_elements) -> structured_buffer_ptr;

	template <typename T>
	inline auto make_structured_buffer(
		renderer_ptr const& rndr,
		resource_usage_mask_t mask,
		resource_storage_t usage,
		structured_buffer_t::typed_shadow_buffer_t<T> const& data) -> structured_buffer_ptr
	{
		return make_structured_buffer(rndr, mask, usage, sizeof(T), data.size(), &data[0], data.size());
	}
}
