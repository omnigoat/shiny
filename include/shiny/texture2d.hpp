#pragma once

#include <shiny/resource.hpp>
#include <shiny/format.hpp>


namespace shiny
{
	struct texture2d_t : resource_t
	{
		texture2d_t(renderer_ptr const& rndr, resource_usage_mask_t usage, format_t format, uint width, uint height, uint mips)
			: resource_t{rndr, resource_type_t::texture2d, usage, resource_storage_t::persistant, element_size(format), width * height}
			, format_{format}
			, width_{width}, height_{height}
			, mips_{mips}
		{}

		auto format() const -> format_t { return format_; }
		auto width() const -> uint { return width_; }
		auto height() const -> uint { return height_; }
		auto mips() const -> uint { return mips_; }

		auto sizeof_host_resource() const -> size_t override { return sizeof(texture2d_t); }

	protected:
		format_t format_ = format_t::unknown;
		uint width_ = 0;
		uint height_ = 0;
		uint mips_ = 0;
	};


	template <typename Device>
	using texture2d_bridge_t = resource_bridge_t<texture2d_t, Device>;
}
