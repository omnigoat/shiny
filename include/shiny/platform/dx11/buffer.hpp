#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/buffer.hpp>
#include <shiny/resource_view.hpp>

#include <shiny/platform/dx11/d3d_fwd.hpp>
#include <shiny/platform/dx11/resource.hpp>

#include <atma/aligned_allocator.hpp>
#include <atma/vector.hpp>


namespace shiny_dx11
{
	struct buffer_t : resource_t
	{
		template <typename T> using aligned_data_t = shiny::buffer_data_t::aligned_data_t<T>;

		buffer_t(
			shiny::renderer_ptr const&,
			shiny::resource_type_t, shiny::resource_usage_mask_t, shiny::resource_storage_t,
			shiny::buffer_dimensions_t const&, shiny::buffer_data_t const&);

		virtual ~buffer_t()
		{}

		auto d3d_buffer() const -> d3d_buffer_ptr const& { return d3d_buffer_; }
		auto d3d_resource() const -> d3d_resource_ptr override { return d3d_buffer_; }

	private:
		d3d_buffer_ptr d3d_buffer_;
	};


	using buffer_bridge_t = shiny::buffer_bridge_t<shiny_dx11::buffer_t>;
	using buffer_bridge_ptr = atma::intrusive_ptr<buffer_bridge_t>;

	//template <typename... Args>
	//inline auto make_buffer(renderer_ptr const& rndr,
	//	resource_type_t rt,
	//	resource_usage_mask_t ru,
	//	resource_storage_t rs,
	//	buffer_dimensions_t const& bdm,
	//	buffer_data_t const& bdt,
	//	Args&&... req_views) -> buffer_ptr
	//{
	//	auto b = atma::make_intrusive<buffer_t>(rndr, rt, ru, rs, bdm, bdt);
	//	int _[] = { 0, (b->bind(req_views), 0)... };
	//	
	//	return b;
	//}

}
