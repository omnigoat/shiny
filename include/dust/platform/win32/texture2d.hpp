#pragma once
//======================================================================
#include <dust/platform/win32/d3d_fwd.hpp>

#include <dust/dust_fwd.hpp>
#include <dust/surface_format.hpp>

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>
//======================================================================
namespace dust
{
	struct texture2d_t : atma::ref_counted
	{
		static auto create(context_ptr const&, surface_format_t, uint width, uint height) -> texture2d_ptr;
		static auto create(context_ptr const&, texture_usage_t, surface_format_t, uint width, uint height) -> texture2d_ptr;

		auto usage() const -> texture_usage_t;
		auto format() const -> surface_format_t;
		auto width() const -> uint;
		auto height() const -> uint;
		auto mips() const -> uint;

		auto d3d_texture() const -> platform::d3d_texture2d_ptr const&;
		auto d3d_texture() -> platform::d3d_texture2d_ptr&;

	private:
		texture2d_t(context_ptr const&, texture_usage_t, surface_format_t, uint width, uint height, uint mips);

	private:
		context_ptr context_;
		texture_usage_t usage_;
		surface_format_t format_;
		uint width_, height_;
		uint mips_;

		platform::d3d_texture2d_ptr d3d_texture_;
	};
}


