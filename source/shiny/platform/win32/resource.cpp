#include <shiny/platform/win32/resource.hpp>
#include <shiny/platform/win32/context.hpp>

using namespace shiny;
using shiny::resource_t;


resource_t::resource_t(context_ptr const& ctx, resource_usage_mask_t usage_flags)
: context_(ctx), usage_flags_(usage_flags), type_(resource_type_t::buffer)
{}

resource_t::resource_t(context_ptr const& ctx, texture_usage_t usage)
: context_(ctx), usage_flags_(), type_(resource_type_t::texture2d)
{
	//usage_flags_ |= resource_usage_t::shader_resource;

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

auto resource_t::make_view(make_view_t const& mv) -> void
{
	auto fmt = platform::dxgi_format_of(mv.element_format);

	views_.push_back(
		atma::make_intrusive_ptr<resource_view_t>(shared_from_this<resource_t>(), (view_type_t)type_, mv.gpu_access, mv.element_format, mv.subset)
	);
}
