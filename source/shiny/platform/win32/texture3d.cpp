#include <shiny/platform/win32/texture3d.hpp>

#include <shiny/platform/win32/dxgid3d_convert.hpp>
#include <shiny/context.hpp>


using namespace shiny;
using shiny::texture3d_t;


auto shiny::make_texture3d(
	context_ptr const& ctx,
	resource_usage_mask_t ru,
	resource_storage_t rs,
	texture3d_dimensions_t const& td) -> texture3d_ptr
{
	return atma::make_intrusive_ptr<texture3d_t>(ctx, ru, rs, td);
}



texture3d_t::texture3d_t(context_ptr const& ctx, resource_usage_mask_t ru, resource_storage_t rs, texture3d_dimensions_t const& td)
	: resource_t(ctx, resource_type_t::texturd3d, ru, rs, element_size(td.format), td.width * td.height * td.depth)
	, format_(td.format), mips_(td.mips), width_(td.width), height_(td.height), depth_(td.depth)
{
	auto const& device = context()->d3d_device();

	auto d3dusage = D3D11_USAGE();
	auto d3dbind = D3D11_BIND_FLAG{};
	auto d3dcpu = D3D11_CPU_ACCESS_FLAG();
	auto d3dfmt = platform::dxgi_format_of(td.format);

	// resource-usage
	if (ru & resource_usage_t::render_target || ru & resource_usage_t::depth_stencil) {
		ATMA_HALT("not possible!");
		return;
	}

	if (ru & resource_usage_t::shader_resource)
		(uint&)d3dbind |= D3D11_BIND_SHADER_RESOURCE;
	if (ru & resource_usage_t::unordered_access)
		(uint&)d3dbind |= D3D11_BIND_UNORDERED_ACCESS;

	// resource-storage
	if (rs == resource_storage_t::immutable) {
		ATMA_HALT("not possible!");	
		return;
	}
	else if (rs == resource_storage_t::transient) {
		d3dusage = D3D11_USAGE_DYNAMIC;
		d3dcpu = D3D11_CPU_ACCESS_WRITE;
	}
	else if (rs == resource_storage_t::staging) {
		d3dusage = D3D11_USAGE_STAGING;
		d3dcpu = D3D11_CPU_ACCESS_READ;
	}

	auto desc = D3D11_TEXTURE3D_DESC{
		(UINT)width_, (UINT)height_, (UINT)depth_, mips_,
		d3dfmt, d3dusage, d3dbind, d3dcpu, 0};

	ATMA_ENSURE_IS(S_OK, device->CreateTexture3D(&desc, nullptr, d3d_texture_.assign()));
}

auto texture3d_t::format() const -> element_format_t
{
	return format_;
}

auto texture3d_t::mips() const -> uint
{
	return mips_;
}

auto texture3d_t::width() const -> size_t
{
	return width_;
}

auto texture3d_t::height() const -> size_t
{
	return height_;
}

auto texture3d_t::depth() const -> size_t
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
