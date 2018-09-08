#pragma once

#include <shiny/buffer.hpp>
#include <shiny/data_declaration.hpp>


namespace shiny
{
	struct vertex_buffer_t : buffer_t
	{
		vertex_buffer_t(renderer_ptr const& rndr, resource_storage_t rs, data_declaration_t const* dd, size_t vertex_count, void const* data, size_t data_vertcount)
			: buffer_t{rndr,
				resource_type_t::vertex_buffer, resource_usage_mask_t::none, rs,
				buffer_dimensions_t{dd->stride(), vertex_count},
				buffer_data_t{data, data_vertcount}}
			, data_declaration_{dd}
			, vertex_count_{data_vertcount}
		{}

		~vertex_buffer_t()
		{}

		auto data_declaration() const -> data_declaration_t const* { return data_declaration_; }
		auto vertex_count() const -> size_t { return vertex_count_; }

		auto sizeof_host_resource() const -> size_t override { return sizeof(vertex_buffer_t); }

	private:
		data_declaration_t const* data_declaration_ = nullptr;
		size_t vertex_count_ = 0;
	};

	template <typename T>
	using vertex_buffer_bridge_t = device_bridge_t<vertex_buffer_t, T>;
}
