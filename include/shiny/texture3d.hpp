#pragma once

#include <shiny/resource.hpp>
#include <shiny/format.hpp>


namespace shiny
{
	struct texture_mipmap_chain_t
	{
		uint mips;
		uint min;
		uint max;
	};

	struct texture3d_t : resource_t
	{
		texture3d_t(renderer_ptr const& rndr, resource_usage_mask_t usage, resource_storage_t storage, format_t format, uint width, uint height, uint depth, uint mips)
			: resource_t{rndr, resource_type_t::texture3d, usage, storage, element_size(format), width * height * depth}
			, format_{format}
			, width_{width}, height_{height}, depth_{depth}
			, mips_{mips}
		{}

		auto format() const -> format_t { return format_; }
		auto width() const -> uint { return width_; }
		auto height() const -> uint { return height_; }
		auto depth() const -> uint { return depth_; }
		auto mips() const -> uint { return mips_; }

		auto sizeof_host_resource() const -> size_t override { return sizeof(texture3d_t); }

	protected:
		format_t format_ = format_t::unknown;
		uint width_ = 0;
		uint height_ = 0;
		uint depth_ = 0;
		uint mips_ = 0;
	};


	template <typename Device>
	using texture3d_bridge_t = resource_bridge_t<texture3d_t, Device>;
}