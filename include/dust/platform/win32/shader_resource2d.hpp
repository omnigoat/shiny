#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/platform/win32/d3d_fwd.hpp>
#include <dust/surface_format.hpp>
//======================================================================
namespace dust
{
	struct shader_resource2d_t : atma::ref_counted
	{
		friend auto create_shader_resource2d(context_ptr const&, view_type_t, surface_format_t, uint width, uint height) -> shader_resource2d_ptr;


		shader_resource2d_t(context_ptr const&, view_type_t, surface_format_t, uint width, uint height);
		~shader_resource2d_t();

		auto view_type() const -> view_type_t { return view_type_; }
		auto backing_texture() const -> texture2d_ptr const& { return texture_; }

	protected:
		context_ptr context_;
		view_type_t view_type_;

		texture2d_ptr texture_;
		platform::d3d_shader_resource_view_ptr d3d_srv_;
		platform::d3d_unordered_access_view_ptr d3d_uav_;
		platform::d3d_view_ptr d3d_view_;
	};
}
