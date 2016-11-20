#include <shiny/platform/win32/texture2d.hpp>

#include <shiny/platform/win32/dxgid3d_convert.hpp>
#include <shiny/context.hpp>


using namespace shiny;
using shiny::texture2d_t;


texture2d_t::texture2d_t(renderer_ptr const& ctx, resource_usage_mask_t usage_flags, format_t format, uint width, uint height, uint mips)
	: resource_t(ctx, resource_type_t::texture2d, usage_flags, resource_storage_t::persistant, shiny::element_size(format), width * height)
	, format_(format), width_(width), height_(height), mips_(mips)
{
	auto const& device = context()->d3d_device();

	auto d3dusage = D3D11_USAGE();
	auto d3dbind = 0;
	auto d3dcpu = D3D11_CPU_ACCESS_FLAG();
	auto d3dfmt = platform::dxgi_format_of(format);

	if (usage_flags & resource_usage_t::render_target || usage_flags & resource_usage_t::depth_stencil)
		if (mips_ == 0)
			mips_ = 1;
		else
			ATMA_ASSERT(mips_ == 1, "render-targets|depth-stencil-targets can not have mipmaps");

	if (usage_flags & resource_usage_t::render_target)
		(uint&)d3dbind |= D3D11_BIND_RENDER_TARGET;
	if (usage_flags & resource_usage_t::depth_stencil)
		(uint&)d3dbind |= D3D11_BIND_DEPTH_STENCIL;
	if (usage_flags & resource_usage_t::shader_resource)
		(uint&)d3dbind |= D3D11_BIND_SHADER_RESOURCE;
	if (usage_flags & resource_usage_t::unordered_access)
		(uint&)d3dbind |= D3D11_BIND_UNORDERED_ACCESS;

	D3D11_TEXTURE2D_DESC texdesc{
		width_, height_, mips_, 1,
		d3dfmt, {1, 0}, d3dusage, (UINT)d3dbind, (UINT)d3dcpu, 0};

	ATMA_ENSURE_IS(S_OK, device->CreateTexture2D(&texdesc, nullptr, d3d_texture_.assign()));
}

texture2d_t::texture2d_t(renderer_ptr const& ctx, platform::d3d_texture2d_ptr const& tx, resource_usage_mask_t rum, format_t f, uint w, uint h, uint m)
	: resource_t{ctx, resource_type_t::texture2d, rum, resource_storage_t::persistant, shiny::element_size(f), w * h}
	, d3d_texture_(tx)
	, format_(f), width_(w), height_(h), mips_(m)
{}

auto texture2d_t::format() const -> format_t
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




auto shiny::make_texture2d(renderer_ptr const& context, resource_usage_mask_t flags, format_t format, uint width, uint height) -> texture2d_ptr
{
	return atma::make_intrusive<texture2d_t>(context, flags, format, width, height, 0u);
}


