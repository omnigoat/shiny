#include <shiny_dx11/texture3d.hpp>

#include <shiny_dx11/dxgid3d_convert.hpp>
#include <shiny/renderer.hpp>


using namespace shiny;
using namespace shiny_dx11;


shiny_dx11::texture3d_t::texture3d_t(renderer_ptr const& rndr, resource_usage_mask_t ru, resource_storage_t rs, format_t format, size_t width, size_t height, size_t depth, uint mips)
{
	auto const& device = rndr->d3d_device();

	auto d3dusage = D3D11_USAGE();
	auto d3dbind = D3D11_BIND_FLAG{};
	auto d3dcpu = D3D11_CPU_ACCESS_FLAG();
	auto d3dfmt = platform::dxgi_format_of(format);

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
		(UINT)width, (UINT)height, (UINT)depth, mips,
		d3dfmt, d3dusage, (UINT)d3dbind, (UINT)d3dcpu, 0};

	ATMA_ENSURE_IS(S_OK, device->CreateTexture3D(&desc, nullptr, d3d_texture_.assign()));
}

