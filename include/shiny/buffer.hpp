#pragma once

#include <shiny/resource.hpp>
#include <shiny/format.hpp>


namespace shiny
{
	struct buffer_dimensions_t
	{
		template <typename T>
		using aligned_data_t = atma::vector<T, atma::aligned_allocator_t<T, 16>>;

		buffer_dimensions_t(size_t stride, size_t count)
			: stride(stride), count(count)
		{
		}

		size_t stride;
		size_t count;

		template <typename T>
		static auto infer(aligned_data_t<T> const& memory) -> buffer_dimensions_t
		{
			return buffer_dimensions_t{sizeof(T), memory.size()};
		}
	};

	struct buffer_data_t
	{
		template <typename T>
		using aligned_data_t = atma::vector<T, atma::aligned_allocator_t<T, 16>>;

		buffer_data_t()
			: data(), size()
		{}

		buffer_data_t(void const* data, size_t size)
			: data(data), size(size)
		{}

		template <typename T>
		buffer_data_t(aligned_data_t<T> const& x)
			: data(x.data()), size(x.size())
		{}

		void const* data;
		size_t size;
	};

	// buffers are specific resource in apis even if the host implementation doesn't do anything
	struct buffer_t : resource_t
	{
		template <typename T>
		using aligned_data_t = buffer_data_t::aligned_data_t<T>;

		buffer_t(
			renderer_ptr const& rndr, resource_type_t rt, resource_usage_mask_t ru,
			resource_storage_t rs, buffer_dimensions_t const& bd, buffer_data_t const&)
			: resource_t{rndr, rt, ru, rs, bd.stride, bd.count}
		{}
		
		virtual ~buffer_t()
		{}

		auto sizeof_host_resource() const -> size_t override { return sizeof(buffer_t); }
	};

	template <typename T>
	using buffer_bridge_t = resource_bridge_t<buffer_t, T>;
}
