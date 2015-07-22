#pragma once

#include <shiny/platform/win32/d3d_fwd.hpp>
#include <shiny/shiny_fwd.hpp>
#include <shiny/resource_view.hpp>

#include <atma/intrusive_ptr.hpp>

#include <vector>


namespace shiny
{
	struct resource_t : atma::ref_counted
	{
		resource_t(context_ptr const&, resource_type_t, resource_usage_mask_t, resource_storage_t, size_t element_stride, size_t element_count);
		virtual ~resource_t();

		auto context() const -> context_ptr const& { return context_; }
		auto resource_type() const -> resource_type_t { return resource_type_; }
		auto resource_usage() const -> resource_usage_mask_t { return resource_usage_; }
		auto resource_storage() const -> resource_storage_t { return resource_storage_; }
		auto elements_stride() const -> size_t { return element_stride_; }
		auto elements_count() const -> size_t { return element_count_; }
		auto resource_size() const -> size_t { return element_stride_ * element_count_; }

		virtual auto d3d_resource() const -> platform::d3d_resource_ptr = 0;

	private:
		context_ptr context_;
		resource_type_t resource_type_;
		resource_usage_mask_t resource_usage_;
		resource_storage_t resource_storage_;
		size_t element_stride_;
		size_t element_count_;
	};
}
