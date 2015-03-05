#include <shiny/platform/win32/texture3d.hpp>

#include <shiny/platform/win32/dxgid3d_convert.hpp>
#include <shiny/context.hpp>


using namespace shiny;
using shiny::texture3d_t;




auto shiny::create_texture3d(context_ptr const& context, texture_usage_t usage, element_format_t format, uint width, uint height, uint depth, uint mips) -> texture3d_ptr
{
	return texture3d_ptr(new texture3d_t(context, usage, format, width, height, depth, mips));
}

auto shiny::create_texture3d(context_ptr const& context, texture_usage_t usage, element_format_t format, uint width, uint mips) -> texture3d_ptr
{
	return create_texture3d(context, usage, format, width, width, width, mips);
}




texture3d_t::texture3d_t(context_ptr const& ctx, texture_usage_t usage, element_format_t format, uint width, uint height, uint depth, uint mips)
: resource_t(ctx, usage), format_(format), mips_(mips), width_(width), height_(height), depth_(depth)
{
	auto const& device = context()->d3d_device();

	auto d3dusage = D3D11_USAGE();
	auto d3dbind = D3D11_BIND_SHADER_RESOURCE;
	auto d3dcpu = D3D11_CPU_ACCESS_FLAG();
	auto d3dfmt = platform::dxgi_format_of(format);

	switch (usage)
	{
		case texture_usage_t::render_target:
		case texture_usage_t::depth_stencil:
			ATMA_HALT("not possible!");
			break;

		case texture_usage_t::immutable:
			ATMA_HALT("not possible!");	
			d3dusage = D3D11_USAGE_IMMUTABLE;
			break;

		case texture_usage_t::streaming:
			d3dusage = D3D11_USAGE_DYNAMIC;
			d3dcpu = D3D11_CPU_ACCESS_WRITE;
			break;
	}

	auto desc = D3D11_TEXTURE3D_DESC{
		width_, height_, depth_, 1,
		d3dfmt, d3dusage, d3dbind, d3dcpu, 0};

	ATMA_ENSURE_IS(S_OK, device->CreateTexture3D(&desc, nullptr, d3d_texture_.assign()));

	ATMA_ENSURE_IS(S_OK, device->CreateShaderResourceView(d3d_texture_.get(), nullptr, d3d_srv_.assign()));
}

auto texture3d_t::format() const -> element_format_t
{
	return format_;
}

auto texture3d_t::mips() const -> uint
{
	return mips_;
}

auto texture3d_t::width() const -> uint
{
	return width_;
}

auto texture3d_t::height() const -> uint
{
	return height_;
}

auto texture3d_t::depth() const -> uint
{
	return depth_;
}

auto texture3d_t::d3d_texture() const -> platform::d3d_texture3d_ptr const&
{
	return d3d_texture_;
}

auto texture3d_t::d3d_texture() -> platform::d3d_texture3d_ptr&
{
	return d3d_texture_;
}

auto texture3d_t::d3d_resource() const -> platform::d3d_resource_ptr
{
	return d3d_texture_;
}

auto texture3d_t::d3d_srv() const -> platform::d3d_shader_resource_view_ptr const&
{
	return d3d_srv_;
}