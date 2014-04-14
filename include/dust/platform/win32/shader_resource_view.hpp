#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/platform/win32/d3d_fwd.hpp>
//======================================================================
namespace dust
{
	struct shader_resource_view_t : atma::ref_counted
	{
		shader_resource_view_t(context_ptr const&, view_type_t, void const* );
		virtual ~shader_resource_view_t();

	protected:
		context_ptr context_;

		platform::d3d_shader_resource_view_ptr d3d_srv_;
	};
}
