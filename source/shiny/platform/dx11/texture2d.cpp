#include <shiny/platform/dx11/texture2d.hpp>

#include <shiny/platform/dx11/dxgid3d_convert.hpp>
#include <shiny/renderer.hpp>

using namespace shiny;
using namespace shiny_dx11;

shiny_dx11::texture2d_t::texture2d_t(renderer_ptr const& rndr, resource_usage_mask_t usage_flags, resource_storage_t rs, format_t format, uint width, uint height, uint mips)
{
	auto const& device = rndr->d3d_device();

	auto d3dusage = D3D11_USAGE();
	auto d3dbind = 0;
	auto d3dcpu = D3D11_CPU_ACCESS_FLAG();
	auto d3dfmt = platform::dxgi_format_of(format);

	if (usage_flags & resource_usage_t::render_target || usage_flags & resource_usage_t::depth_stencil)
		if (mips == 0)
			mips = 1;
		else
			ATMA_ASSERT(mips == 1, "render-targets|depth-stencil-targets can not have mipmaps");

	if (usage_flags & resource_usage_t::render_target)
		(uint&)d3dbind |= D3D11_BIND_RENDER_TARGET;
	if (usage_flags & resource_usage_t::depth_stencil)
		(uint&)d3dbind |= D3D11_BIND_DEPTH_STENCIL;
	if (usage_flags & resource_usage_t::shader_resource)
		(uint&)d3dbind |= D3D11_BIND_SHADER_RESOURCE;
	if (usage_flags & resource_usage_t::unordered_access)
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


	D3D11_TEXTURE2D_DESC texdesc{
		width, height, mips, 1,
		d3dfmt, {1, 0}, d3dusage, (UINT)d3dbind, (UINT)d3dcpu, 0};

	ATMA_ENSURE_IS(S_OK, device->CreateTexture2D(&texdesc, nullptr, d3d_texture_.assign()));
}

shiny_dx11::texture2d_t::texture2d_t(platform::d3d_texture2d_ptr const& tx)
	: d3d_texture_(tx)
{}


