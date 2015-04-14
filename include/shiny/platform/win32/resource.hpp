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
		resource_t(context_ptr const& ctx, resource_usage_mask_t usage_flags);
		resource_t(context_ptr const& ctx, texture_usage_t usage);

		virtual ~resource_t();

		auto context() const -> context_ptr const& { return context_; }
		auto usage_flags() const -> resource_usage_mask_t { return usage_flags_; }
		auto resource_type() const -> resource_type_t { return type_; }
		virtual auto element_count() const -> uint = 0;

		virtual auto d3d_resource() const -> platform::d3d_resource_ptr = 0;

	private:
		context_ptr context_;
		resource_usage_mask_t usage_flags_;
		resource_type_t type_;
	};
}
