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

	protected:
		context_ptr context_;
		view_type_t view_type_;

		texture2d_ptr texture_;
		platform::d3d_shader_resource_view_ptr d3d_srv_;
	};
}
