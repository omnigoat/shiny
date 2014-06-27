#include <dust/platform/win32/resource.hpp>
#include <dust/platform/win32/context.hpp>

using namespace dust;
using dust::resource_t;

resource_t::resource_t(context_ptr const& ctx, resource_usage_flags_t usage_flags)
: context_(ctx), usage_flags_(usage_flags)
{}

resource_t::resource_t(context_ptr const& ctx, texture_usage_t usage)
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

resource_t::~resource_t()
{
}
