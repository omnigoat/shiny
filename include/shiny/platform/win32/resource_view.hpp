#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/element_format.hpp>
#include <shiny/resource.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <atma/intrusive_ptr.hpp>


namespace shiny
{
	struct resource_view_t : atma::ref_counted
	{
		resource_view_t(resource_cptr const&, gpu_access_t, element_format_t, uint offset, uint count);

		auto context() const -> context_ptr const& { return resource_->context(); }
		auto resource() const -> resource_cptr const& { return resource_; }
		auto gpu_access() const -> gpu_access_t { return gpu_access_; }
		auto format() const -> element_format_t { return format_; }

		auto d3d_view() const -> platform::d3d_view_ptr const& { return d3d_view_; }

	private:
		resource_cptr resource_;
		gpu_access_t gpu_access_;
		element_format_t format_;
		uint offset_, count_;

		platform::d3d_view_ptr d3d_view_;
		platform::d3d_shader_resource_view_ptr d3d_srv_;
		platform::d3d_unordered_access_view_ptr d3d_uav_;
	};
}
