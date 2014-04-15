#include <dust/platform/win32/shader_resource2d.hpp>

#include <dust/context.hpp>
#include <dust/texture2d.hpp>

#include <atma/assert.hpp>


using namespace dust;
using dust::shader_resource2d_t;

auto dust::create_shader_resource2d(context_ptr const& ctx, view_type_t view_type, surface_format_t sf, uint width, uint height) -> dust::shader_resource2d_ptr
{
	return dust::shader_resource2d_ptr(new shader_resource2d_t(ctx, view_type, sf, width, height));
}

shader_resource2d_t::shader_resource2d_t(context_ptr const& ctx, view_type_t view_type, surface_format_t sf, uint width, uint height)
	: context_(ctx), view_type_(view_type)
{
	texture_ = dust::create_texture2d(context_, resource_usage_t::shader_resource, sf, width, height);
	ATMA_ASSERT(texture_);


	auto srvd = D3D11_SHADER_RESOURCE_VIEW_DESC();
	srvd.Format = DXGI_FORMAT_R32_FLOAT;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MipLevels = -1;
	srvd.Texture2D.MostDetailedMip = 0;

	context_->d3d_device()->CreateShaderResourceView(texture_->d3d_texture().get(), &srvd, d3d_srv_.assign());
}

shader_resource2d_t::~shader_resource2d_t()
{
}

