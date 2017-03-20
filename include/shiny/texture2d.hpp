#pragma once

#include <atma/config/platform.hpp>

#include <shiny/resource.hpp>
//#include <shiny/api_resource.hpp>


namespace shiny
{
	struct texture2d_t : resource_t
	{
		texture2d_t(renderer_ptr const& rndr, resource_usage_mask_t usage, resource_storage_t storage, format_t format, uint width, uint height, uint mips)
			: resource_t{rndr,
				resource_type_t::texture2d,
				usage, storage,
				element_size(format), element_count(format)}
			, format_{format}
			, width_{width}, height_{height}, mips_{mips}
		{}

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
