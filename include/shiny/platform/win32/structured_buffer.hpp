#pragma once

#include <shiny/buffer.hpp>
#include <shiny/platform/win32/dxgid3d_convert.hpp>


namespace shiny
{
	struct structured_buffer_t : shiny::buffer_t
	{
		structured_buffer_t(context_ptr const&, resource_usage_mask_t const&, buffer_usage_t usage, size_t stride, uint elements, void const* data, uint data_elemcount);
	};

	auto make_structured_buffer(
		context_ptr const&,
		resource_usage_mask_t,
		buffer_usage_t,
		size_t stride, uint elements,
		void const* data, uint data_elements) -> structured_buffer_ptr;

	template <typename T>
	inline auto make_structured_buffer(
		context_ptr const& ctx,
		resource_usage_mask_t mask,
		buffer_usage_t usage,
		structured_buffer_t::typed_shadow_buffer_t<T> const& data) -> structured_buffer_ptr
	{
		return make_structured_buffer(ctx, mask, usage, sizeof(T), data.size(), &data[0], data.size());
	}
}
