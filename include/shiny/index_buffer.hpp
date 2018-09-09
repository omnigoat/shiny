#pragma once

#include <shiny/format.hpp>
#include <shiny/buffer.hpp>


namespace shiny
{
	struct index_buffer_t : buffer_t
	{
		index_buffer_t(renderer_ptr const& rndr, resource_storage_t rs, format_t format, uint index_count, void const* data, uint data_indexcount)
			: buffer_t{rndr,
				resource_type_t::index_buffer, resource_usage_mask_t::none, rs,
				buffer_dimensions_t{element_size(format), index_count},
				buffer_data_t{data, data_indexcount}}
			, index_format_{format}
			, index_count_{index_count}
		{}

		auto index_format() const -> format_t { return index_format_; }
		auto index_count() const -> uint { return index_count_; }

		auto sizeof_host_resource() const -> size_t override { return sizeof(index_buffer_t); }

	private:
		format_t index_format_ = format_t::unknown;
		uint index_count_ = 0;
	};

	template <typename T>
	using index_buffer_bridge_t = device_bridge_t<index_buffer_t, T>;
}