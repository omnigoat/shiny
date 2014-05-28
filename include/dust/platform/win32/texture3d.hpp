#pragma once
//======================================================================
#include <dust/platform/win32/d3d_fwd.hpp>

#include <dust/dust_fwd.hpp>
#include <dust/resource.hpp>
#include <dust/surface_format.hpp>

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>
//======================================================================
namespace dust
{
	struct texture3d_t : resource_t
	{
		friend auto create_texture3d(context_ptr const&, surface_format_t, uint mips, uint width, uint height, uint depth) -> texture3d_ptr;
		friend auto create_texture3d(context_ptr const&, surface_format_t, uint mips, uint width) -> texture3d_ptr;

		auto format() const -> surface_format_t;
		auto mips() const -> uint;
		auto width() const -> uint;
		auto height() const -> uint;
		auto depth() const -> uint;

		//auto lock() -> 

		auto d3d_texture() const -> platform::d3d_texture3d_ptr const&;
		auto d3d_texture() -> platform::d3d_texture3d_ptr&;

		auto d3d_resource() const -> platform::d3d_resource_ptr override;

	private:
		texture3d_t(context_ptr const&, resource_usage_flags_t, surface_format_t, uint mips, uint width, uint height, uint depth);

	private:
		surface_format_t format_;
		uint mips_;
		uint width_, height_, depth_;

		platform::d3d_texture3d_ptr d3d_texture_;
	};


	inline auto create_texture3d(context_ptr const& context, surface_format_t format, uint mips, uint width) -> texture3d_ptr
	{
		return create_texture3d(context, format, mips, width, width, width);
	}
}
