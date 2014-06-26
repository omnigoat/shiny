#include <dust/platform/win32/texture2d.hpp>

#include <dust/platform/win32/dxgid3d_convert.hpp>
#include <dust/context.hpp>


using namespace dust;
using dust::texture2d_t;


//======================================================================
// create_texture2d
//======================================================================
auto dust::create_texture2d(context_ptr const& context, resource_usage_flags_t flags, element_format_t format, uint width, uint height) -> texture2d_ptr
{
	return texture2d_ptr(new texture2d_t(context, flags, format, 0, width, height));
}

auto dust::create_texture2d(context_ptr const& context, element_format_t format, uint width, uint height) -> texture2d_ptr
{
	return create_texture2d(context, {}, format, width, height);
}


//======================================================================
// texture2d
//======================================================================
texture2d_t::texture2d_t(context_ptr const& ctx, resource_usage_flags_t usage_flags, element_format_t format, uint width, uint height, uint mips)
: resource_t(ctx, usage_flags), format_(format), width_(width), height_(height), mips_(mips)
{
	auto const& device = context()->d3d_device();

	auto const miplevels =
		(usage_flags & resource_usage_t::render_target) ? 1 :
		(usage_flags & resource_usage_t::depth_stencil) ? 1 :
		mips_;

	auto bind_flags = D3D11_BIND_FLAG();
	if (usage_flags & resource_usage_t::render_target)
		(uint&)bind_flags |= D3D11_BIND_RENDER_TARGET;
	if (usage_flags & resource_usage_t::depth_stencil)
		(uint&)bind_flags |= D3D11_BIND_DEPTH_STENCIL;
	if (usage_flags & resource_usage_t::shader_resource)
		(uint&)bind_flags |= D3D11_BIND_SHADER_RESOURCE;
	if (usage_flags & resource_usage_t::unordered_access)
		(uint&)bind_flags |= D3D11_BIND_UNORDERED_ACCESS;

	D3D11_TEXTURE2D_DESC texdesc{
		width_, height_, miplevels, 1,
		platform::dxgi_format_of(format), {1, 0}, D3D11_USAGE_DEFAULT, bind_flags,
		0, 0};

	ATMA_ENSURE_IS(S_OK, device->CreateTexture2D(&texdesc, nullptr, d3d_texture_.assign()));

//auto desc = D3D11_SHADER_RESOURCE_VIEW_DESC{};
	//desc.Format = platform::dxgi_format_of(format);
	//desc.Texture2D = {0, -1};

	ATMA_ENSURE_IS(S_OK, device->CreateShaderResourceView(d3d_texture_.get(), nullptr, d3d_srv_.assign()));
}

auto texture2d_t::format() const -> element_format_t
{
	return format_;
}

auto texture2d_t::width() const -> uint
{
	return width_;
}

auto texture2d_t::height() const -> uint
{
	return height_;
}

auto texture2d_t::mips() const -> uint
{
	return mips_;
}

auto texture2d_t::d3d_texture() const -> platform::d3d_texture2d_ptr const&
{
	return d3d_texture_;
}

auto texture2d_t::d3d_texture() -> platform::d3d_texture2d_ptr&
{
	return d3d_texture_;
}

auto texture2d_t::d3d_resource() const -> platform::d3d_resource_ptr
{
	return d3d_texture_;
}

auto texture2d_t::d3d_srv() const -> platform::d3d_shader_resource_view_ptr const&
{
	return d3d_srv_;
}
