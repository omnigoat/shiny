#pragma once
//======================================================================
#include <dust/platform/win32/d3d_fwd.hpp>
#include <dust/dust_fwd.hpp>
//======================================================================
namespace dust
{
	struct resource_t : atma::ref_counted
	{
		resource_t(context_ptr const& ctx, resource_usage_flags_t usage_flags)
			: context_(ctx), usage_flags_(usage_flags)
		{}

		resource_t(context_ptr const& ctx, texture_usage_t usage)
			: context_(ctx), usage_flags_()
		{
			usage_flags_ |= resource_usage_t::shader_resource;

			switch (usage)
			{
				case texture_usage_t::render_target:
					usage_flags_ |= resource_usage_t::render_target;
					break;

				case texture_usage_t::depth_stencil:
					usage_flags_ |= resource_usage_t::depth_stencil;
					break;
			}
		}

		virtual ~resource_t()
		{}

		auto context() const -> context_ptr const& { return context_; }
		auto usage_flags() const -> resource_usage_flags_t { return usage_flags_; }

		virtual auto d3d_resource() const -> platform::d3d_resource_ptr = 0;

	private:
		context_ptr context_;
		resource_usage_flags_t usage_flags_;
	};

	struct mapped_subresource_t
	{
		void const* data;
		uint size;
	};
}
