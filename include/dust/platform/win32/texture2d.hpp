#pragma once
//======================================================================
#include <dust/platform/win32/d3d_fwd.hpp>

#include <dust/dust_fwd.hpp>
#include <dust/resource.hpp>
#include <dust/element_format.hpp>

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>
//======================================================================
namespace dust
{
	struct texture2d_t : resource_t
	{
		friend auto create_texture2d(context_ptr const&, resource_usage_flags_t, element_format_t, uint width, uint height) -> texture2d_ptr;
		friend auto create_texture2d(context_ptr const&, element_format_t, uint width, uint height) -> texture2d_ptr;
		
		auto format() const -> element_format_t;
		auto width() const -> uint;
		auto height() const -> uint;
		auto mips() const -> uint;

		auto d3d_texture() const -> platform::d3d_texture2d_ptr const&;
		auto d3d_texture() -> platform::d3d_texture2d_ptr&;

		auto d3d_resource() const -> platform::d3d_resource_ptr override;
		auto d3d_srv() const -> platform::d3d_shader_resource_view_ptr const& override;

	private:
		texture2d_t(context_ptr const&, resource_usage_flags_t, element_format_t, uint width, uint height, uint mips);

	private:
		element_format_t format_;
		uint width_, height_;
		uint mips_;

		platform::d3d_texture2d_ptr d3d_texture_;
		platform::d3d_shader_resource_view_ptr d3d_srv_;
	};
}


