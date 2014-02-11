#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/platform/win32/d3d_fwd.hpp>
//======================================================================
namespace dust {

	struct pixel_shader_t : atma::ref_counted
	{
		pixel_shader_t(context_ptr const&);

		auto d3d_ps() const -> platform::d3d_pixel_shader_ptr const& { return d3d_ps_; }

	private:
		context_ptr context_;
		platform::d3d_pixel_shader_ptr d3d_ps_;
	};

}


