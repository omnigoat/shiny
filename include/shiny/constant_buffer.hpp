#pragma once

#include <shiny/buffer.hpp>


namespace shiny
{
	struct constant_buffer_t : buffer_t
	{
		constant_buffer_t(renderer_ptr const& renderer, void const* data, size_t data_size)
			: buffer_t(renderer,
				resource_type_t::constant_buffer, resource_usage_mask_t::none, resource_storage_t::constant,
				buffer_dimensions_t{1, data_size}, buffer_data_t{data, 1})
		{}

		auto sizeof_host_resource() const -> size_t override { return sizeof(constant_buffer_t); }
	};

	template <typename T>
	using constant_buffer_bridge_t = device_bridge_t<constant_buffer_t, T>;
}
