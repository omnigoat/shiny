#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/element_format.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <atma/intrusive_ptr.hpp>


namespace shiny
{
	struct resource_subset_t
	{
		resource_subset_t() = default;

		resource_subset_t(size_t offset, size_t count)
			: offset(offset), count(count)
		{
		}

		size_t offset = 0;
		size_t count  = 0;

		static const resource_subset_t whole;
	};

	// for textures
	using generate_default_view_t = resource_subset_t;




	struct resource_view_t : atma::ref_counted
	{
		resource_view_t(resource_cptr const&, resource_view_type_t, element_format_t, resource_subset_t);

		auto context() const -> context_ptr const&;
		auto resource() const -> resource_cptr const& { return resource_; }
		auto view_type() const -> resource_view_type_t { return resource_view_type_; }
		auto format() const -> element_format_t { return format_; }
		auto subset() const -> resource_subset_t const& { return subset_; }

		auto d3d_view() const -> platform::d3d_view_ptr const& { return d3d_view_; }

	private:
		resource_cptr resource_;
		resource_view_type_t resource_view_type_;
		element_format_t format_;
		resource_subset_t subset_;

		platform::d3d_view_ptr d3d_view_;
		platform::d3d_shader_resource_view_ptr d3d_srv_;
		platform::d3d_unordered_access_view_ptr d3d_uav_;
	};

#if 0
	struct structured_resource_view_t : resource_view_t
	{
		structured_resource_view_t(resource_cptr const&, )
	};
#endif

	auto make_resource_view(resource_cptr const&, resource_view_type_t, element_format_t, resource_subset_t = resource_subset_t::whole) -> resource_view_ptr;
}
