#include <shiny/platform/win32/shader_resource2d.hpp>

#include <shiny/context.hpp>
#include <shiny/texture2d.hpp>

#include <atma/assert.hpp>


using namespace shiny;
using shiny::shader_resource2d_t;

auto shiny::create_shader_resource2d(context_ptr const& ctx, view_type_t view_type, element_format_t sf, uint width, uint height) -> shiny::shader_resource2d_ptr
{
	return shiny::shader_resource2d_ptr(new shader_resource2d_t(ctx, view_type, sf, width, height));
}

shader_resource2d_t::shader_resource2d_t(context_ptr const& ctx, view_type_t view_type, element_format_t sf, uint width, uint height)
	: context_(ctx), view_type_(view_type)
{
#if 0
	texture_ = shiny::create_texture2d(context_, {resource_usage_t::shader_resource, resource_usage_t::unordered_access}, sf, width, height);
	ATMA_ASSERT(texture_);

	auto srvd = D3D11_SHADER_RESOURCE_VIEW_DESC();

	srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MipLevels = -1;
	srvd.Texture2D.MostDetailedMip = 0;

	if (view_type_ == view_type_t::read_only) {
		ATMA_ENSURE_IS(S_OK, context_->d3d_device()->CreateShaderResourceView(texture_->d3d_texture().get(), nullptr, d3d_srv_.assign()));
		d3d_view_ = d3d_srv_;
	}
	else {
		ATMA_ENSURE_IS(S_OK, context_->d3d_device()->CreateUnorderedAccessView(texture_->d3d_texture().get(), nullptr, d3d_uav_.assign()));
		d3d_view_ = d3d_uav_;
	}
#endif
}

shader_resource2d_t::~shader_resource2d_t()
{
}

