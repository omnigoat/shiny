#pragma once
//======================================================================
#include <shiny/dust_fwd.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>
#include <shiny/element_format.hpp>
//======================================================================
namespace shiny
{
	struct shader_resource2d_t : atma::ref_counted
	{
		friend auto create_shader_resource2d(context_ptr const&, view_type_t, element_format_t, uint width, uint height) -> shader_resource2d_ptr;


		shader_resource2d_t(context_ptr const&, view_type_t, element_format_t, uint width, uint height);
		~shader_resource2d_t();

		auto view_type() const -> view_type_t { return view_type_; }
		auto backing_texture() const -> texture2d_ptr const& { return texture_; }
		auto d3d_view() const -> platform::d3d_view_ptr const& { return d3d_view_; }

	protected:
		context_ptr context_;
		view_type_t view_type_;

		texture2d_ptr texture_;
		platform::d3d_shader_resource_view_ptr d3d_srv_;
		platform::d3d_unordered_access_view_ptr d3d_uav_;
		platform::d3d_view_ptr d3d_view_;
	};
}
