#pragma once

#include <atma/config/platform.hpp>

//#include <shiny/api_resource.hpp>


namespace shiny
{
	struct texture2d_t
	{
		texture2d_t(renderer_ptr const&, resource_usage_mask_t, format_t, uint width, uint height, uint mips);
		//texture2d_t(renderer_ptr const&, resource_usage_mask_t, format_t, uint width, uint height, uint mips, resource_subset_t);

		auto format() const -> format_t { return format_; }
		auto width() const -> uint { return width_; }
		auto height() const -> uint { return height_; }
		auto mips() const -> uint { return mips_; }

	protected:
		format_t format_ = format_t::unknown;
		uint width_ = 0;
		uint height_ = 0;
		uint mips_ = 0;
	};
}

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/dx11/texture2d.hpp>
#endif
